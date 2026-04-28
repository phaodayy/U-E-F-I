#include "hyper_process.hpp"

#include "hypercall_bridge.hpp"
#include <protec/skCrypt.h>

#include <TlHelp32.h>
#include <winternl.h>

#include <memory>
#include <winuser.h>
#include <iostream>
#include <vector>

typedef struct _HANDLEENTRY {
    PVOID  phead;    // 0x00
    PVOID  pOwner;   // 0x08
    BYTE   bType;    // 0x10
    BYTE   bFlags;   // 0x11
    WORD   wUnused;  // 0x12
    DWORD  wUnused2; // 0x14 - Padding/Extra to make it 24 bytes total
} HANDLEENTRY, * PHANDLEENTRY;

typedef struct _SERVERINFO {
    DWORD dwSRVIFlags;
    DWORD cHandleEntries; // Usually DWORD
} SERVERINFO, * PSERVERINFO;

typedef struct _SHAREDINFO {
    PSERVERINFO  psi;
    PHANDLEENTRY aheList;
} SHAREDINFO, * PSHAREDINFO;


namespace
{
    constexpr auto kSystemModuleInformationClass = static_cast<SYSTEM_INFORMATION_CLASS>(11);


    namespace eprocess
    {
        constexpr std::uint64_t UniqueProcessId = 0x440;
        constexpr std::uint64_t ActiveProcessLinks = 0x448;
        constexpr std::uint64_t DirectoryTableBase = 0x28;
        constexpr std::uint64_t SectionBaseAddress = 0x520;
        constexpr std::uint64_t Peb = 0x550;
    }

    using NtQuerySystemInformation_t = NTSTATUS(NTAPI*)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);

    typedef struct _RTL_PROCESS_MODULE_INFORMATION_LOCAL
    {
        HANDLE Section;
        PVOID MappedBase;
        PVOID ImageBase;
        ULONG ImageSize;
        ULONG Flags;
        USHORT LoadOrderIndex;
        USHORT InitOrderIndex;
        USHORT LoadCount;
        USHORT OffsetToFileName;
        UCHAR FullPathName[256];
    } RTL_PROCESS_MODULE_INFORMATION_LOCAL;

    typedef struct _RTL_PROCESS_MODULES_LOCAL
    {
        ULONG NumberOfModules;
        RTL_PROCESS_MODULE_INFORMATION_LOCAL Modules[1];
    } RTL_PROCESS_MODULES_LOCAL;

    std::uint64_t g_CurrentCr3 = 0;
    std::uint64_t g_PsInitialSystemProcess = 0;
    std::uint64_t g_ImageFileNameOffset = 0x5a8; // Auto-calibrated

    template <typename value_type>
    bool ReadKernelValue(const std::uint64_t address, value_type* value)
    {
        if (value == nullptr || g_CurrentCr3 == 0)
        {
            return false;
        }

        return telemetryHyperCall::ReadGuestVirtualMemory(value, address, g_CurrentCr3, sizeof(value_type)) == sizeof(value_type);
    }

    std::string ResolveKernelImagePath(const char* raw_path)
    {
        if (raw_path == nullptr || raw_path[0] == '\0')
        {
            return {};
        }

        std::string image_path(raw_path);

        const std::string system_root_prefix = skCrypt("\\SystemRoot\\");
        const std::string dos_device_prefix = skCrypt("\\??\\");

        if (image_path.rfind(system_root_prefix, 0) == 0)
        {
            char windows_directory[MAX_PATH] = {};
            if (GetWindowsDirectoryA(windows_directory, MAX_PATH) == 0)
            {
                return {};
            }

            return std::string(windows_directory) + skCrypt("\\") + image_path.substr(system_root_prefix.size());
        }

        if (image_path.rfind(dos_device_prefix, 0) == 0)
        {
            return image_path.substr(dos_device_prefix.size());
        }

        return image_path;
    }

    bool ResolvePsInitialSystemProcess()
    {
        if (g_CurrentCr3 != 0 && g_PsInitialSystemProcess != 0)
        {
            return true;
        }

        g_CurrentCr3 = telemetryHyperCall::ReadGuestCr3();
        if (g_CurrentCr3 == 0)
        {
            return false;
        }

        auto* ntdll = GetModuleHandleA(skCrypt("ntdll.dll"));
        if (ntdll == nullptr)
        {
            return false;
        }

        const auto nt_query_system_information = reinterpret_cast<NtQuerySystemInformation_t>(
            GetProcAddress(ntdll, skCrypt("NtQuerySystemInformation")));
        if (nt_query_system_information == nullptr)
        {
            return false;
        }

        ULONG bytes_needed = 0;
        NTSTATUS status = nt_query_system_information(kSystemModuleInformationClass, nullptr, 0, &bytes_needed);
        if (bytes_needed == 0 && status < 0)
        {
            return false;
        }

        auto buffer = std::make_unique<std::uint8_t[]>(bytes_needed);
        status = nt_query_system_information(kSystemModuleInformationClass, buffer.get(), bytes_needed, &bytes_needed);
        if (status < 0)
        {
            return false;
        }

        const auto* modules = reinterpret_cast<const RTL_PROCESS_MODULES_LOCAL*>(buffer.get());
        if (modules->NumberOfModules == 0)
        {
            return false;
        }

        const auto& ntoskrnl = modules->Modules[0];
        const auto kernel_base = reinterpret_cast<std::uint64_t>(ntoskrnl.ImageBase);
        const auto image_path = ResolveKernelImagePath(reinterpret_cast<const char*>(ntoskrnl.FullPathName));


        if (kernel_base == 0 || image_path.empty())
        {
            return false;
        }

        HMODULE local_ntoskrnl = LoadLibraryExA(image_path.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
        if (local_ntoskrnl == nullptr)
        {
            return false;
        }

        // STEALTH: Manual export resolution without GetProcAddress for sensitive kernel variables
        auto GetExportRVA = [](HMODULE module, const std::string& name) -> std::uintptr_t {
            auto* base = reinterpret_cast<std::uint8_t*>(module);
            auto* dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
            auto* nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
            auto* export_dir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(base + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
            auto* names = reinterpret_cast<std::uint32_t*>(base + export_dir->AddressOfNames);
            auto* ordinals = reinterpret_cast<std::uint16_t*>(base + export_dir->AddressOfNameOrdinals);
            auto* functions = reinterpret_cast<std::uint32_t*>(base + export_dir->AddressOfFunctions);

            for (std::uint32_t i = 0; i < export_dir->NumberOfNames; i++) {
                if (name == reinterpret_cast<const char*>(base + names[i])) {
                    return functions[ordinals[i]];
                }
            }
            return 0;
        };

        const std::uintptr_t export_rva = GetExportRVA(local_ntoskrnl, skCrypt("PsInitialSystemProcess"));
        if (export_rva == 0)
        {
            FreeLibrary(local_ntoskrnl);
            return false;
        }
        FreeLibrary(local_ntoskrnl);

        g_PsInitialSystemProcess = kernel_base + export_rva;
 
        // --- DYNAMIC OFFSET CALIBRATION (SYSTEM PROCESS SCAN) ---
        // Scan EPROCESS of PID 4 for "System" to find ImageFileName offset
        std::uint64_t system_eprocess = 0;
        if (telemetryHyperCall::ReadGuestVirtualMemory(&system_eprocess, g_PsInitialSystemProcess, g_CurrentCr3, 8) == 8) {
            for (std::uint64_t offset = 0x400; offset < 0x800; offset++) {
                char buf[8] = {};
                if (telemetryHyperCall::ReadGuestVirtualMemory(buf, system_eprocess + offset, g_CurrentCr3, 6) == 6) {
                    if (strcmp(buf, skCrypt("System")) == 0) {
                        g_ImageFileNameOffset = offset;
                        break;
                    }
                }
            }
        }

        return g_PsInitialSystemProcess != 0;
    }

    typedef struct _KLDR_DATA_TABLE_ENTRY_LOCAL
    {
        LIST_ENTRY InLoadOrderLinks;
        PVOID ExceptionTable;
        ULONG ExceptionTableSize;
        PVOID GpValue;
        PVOID NonPagedDebugInfo;
        PVOID DllBase;
        PVOID EntryPoint;
        ULONG SizeOfImage;
        UNICODE_STRING FullDllName;
        UNICODE_STRING BaseDllName;
    } KLDR_DATA_TABLE_ENTRY_LOCAL;

}

std::uint64_t telemetryHyperProcess::GetKernelModuleBase(const char* module_name)
{
    if (!ResolvePsInitialSystemProcess()) return 0;

    // 1. Dùng NtQuerySystemInformation lấy ntoskrnl base (đã làm trong Resolve)
    // Nhưng để "Pure Hypervisor" walk, mình sẽ lấy module list từ ntoskrnl
    
    // Tìm RVA của PsLoadedModuleList
    ULONG bytes_needed = 0;
    const auto ntdll = GetModuleHandleA(skCrypt("ntdll.dll"));
    const auto nt_query_system_information = reinterpret_cast<NtQuerySystemInformation_t>(GetProcAddress(ntdll, skCrypt("NtQuerySystemInformation")));
    
    nt_query_system_information(kSystemModuleInformationClass, nullptr, 0, &bytes_needed);
    auto buffer = std::make_unique<std::uint8_t[]>(bytes_needed);
    nt_query_system_information(kSystemModuleInformationClass, buffer.get(), bytes_needed, &bytes_needed);
    
    const auto* modules_list = reinterpret_cast<const RTL_PROCESS_MODULES_LOCAL*>(buffer.get());
    const auto& ntoskrnl = modules_list->Modules[0];
    const auto kernel_base = reinterpret_cast<std::uint64_t>(ntoskrnl.ImageBase);
    const auto image_path = ResolveKernelImagePath(reinterpret_cast<const char*>(ntoskrnl.FullPathName));

    HMODULE local_ntoskrnl = LoadLibraryExA(image_path.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
    if (!local_ntoskrnl) return 0;

    auto GetExportRVA = [](HMODULE module, const std::string& name) -> std::uintptr_t {
        auto* base = reinterpret_cast<std::uint8_t*>(module);
        auto* dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
        auto* nt = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
        auto* export_dir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(base + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
        auto* names = reinterpret_cast<std::uint32_t*>(base + export_dir->AddressOfNames);
        auto* ordinals = reinterpret_cast<std::uint16_t*>(base + export_dir->AddressOfNameOrdinals);
        auto* functions = reinterpret_cast<std::uint32_t*>(base + export_dir->AddressOfFunctions);
        for (std::uint32_t i = 0; i < export_dir->NumberOfNames; i++) {
            if (name == reinterpret_cast<const char*>(base + names[i])) return functions[ordinals[i]];
        }
        return 0;
    };

    uintptr_t list_rva = GetExportRVA(local_ntoskrnl, skCrypt("PsLoadedModuleList"));
    FreeLibrary(local_ntoskrnl);

    if (list_rva == 0) return 0;

    uint64_t list_head_addr = kernel_base + list_rva;
    LIST_ENTRY list_head;
    if (telemetryHyperCall::ReadGuestVirtualMemory(&list_head, list_head_addr, g_CurrentCr3, sizeof(LIST_ENTRY)) != sizeof(LIST_ENTRY))
        return 0;

    uint64_t current_link = reinterpret_cast<uint64_t>(list_head.Flink);
    int safety_limit = 512;

    while (current_link != list_head_addr && safety_limit-- > 0)
    {
        KLDR_DATA_TABLE_ENTRY_LOCAL entry;
        if (telemetryHyperCall::ReadGuestVirtualMemory(&entry, current_link, g_CurrentCr3, sizeof(KLDR_DATA_TABLE_ENTRY_LOCAL)) == sizeof(KLDR_DATA_TABLE_ENTRY_LOCAL))
        {
            wchar_t base_name_buf[256] = { 0 };
            uint32_t name_len = (entry.BaseDllName.Length < 510) ? entry.BaseDllName.Length : 510;
            
            if (telemetryHyperCall::ReadGuestVirtualMemory(base_name_buf, reinterpret_cast<uint64_t>(entry.BaseDllName.Buffer), g_CurrentCr3, name_len) > 0)
            {
                char current_module_name[256] = { 0 };
                for (uint32_t i = 0; i < name_len / 2; i++) current_module_name[i] = (char)base_name_buf[i];

                if (_stricmp(current_module_name, module_name) == 0)
                {
                    return reinterpret_cast<uint64_t>(entry.DllBase);
                }
            }
            current_link = reinterpret_cast<uint64_t>(entry.InLoadOrderLinks.Flink);
        }
        else break;
    }

    return 0;
}

bool telemetryHyperProcess::Initialize()
{
    return ResolvePsInitialSystemProcess();
}

bool telemetryHyperProcess::QueryProcessData(const std::uint32_t pid, query_process_data_packet* const output)
{
    if (output == nullptr)
    {
        return false;
    }

    if (!ResolvePsInitialSystemProcess())
    {
        return false;
    }

    std::uint64_t current_eprocess = 0;
    std::uint64_t walk_cr3 = g_CurrentCr3;

    if (telemetryHyperCall::ReadGuestVirtualMemory(&current_eprocess, g_PsInitialSystemProcess, walk_cr3, 8) != 8 || current_eprocess == 0)
    {
        return false;
    }

    const auto start_eprocess = current_eprocess;
    
    std::uint32_t best_pid = 0;
    std::uint64_t best_cr3 = 0;
    std::uint64_t best_base = 0;
    std::uint64_t best_peb = 0;
    std::uint64_t max_threads = 0;

    do
    {
        std::uint64_t current_pid = 0;
        std::uint64_t current_cr3 = 0;
        std::uint64_t current_base = 0;
        std::uint64_t current_peb = 0;
        std::uint64_t current_commit = 0;

        telemetryHyperCall::ReadGuestVirtualMemory(&current_pid, current_eprocess + eprocess::UniqueProcessId, walk_cr3, 8);
        telemetryHyperCall::ReadGuestVirtualMemory(&current_cr3, current_eprocess + eprocess::DirectoryTableBase, walk_cr3, 8);
        telemetryHyperCall::ReadGuestVirtualMemory(&current_base, current_eprocess + eprocess::SectionBaseAddress, walk_cr3, 8);
        telemetryHyperCall::ReadGuestVirtualMemory(&current_peb, current_eprocess + eprocess::Peb, walk_cr3, 8);
        
        // EPROCESS + 0x5f0 is ActiveThreads (number of threads in this process)
        // Main telemetry game usually has 50-100+ threads, while child/zombie processes have very few (<10).
        std::uint64_t current_threads = 0;
        telemetryHyperCall::ReadGuestVirtualMemory(&current_threads, current_eprocess + 0x5f0, walk_cr3, 4); // It's a DWORD

        char name[16] = {};
        if (telemetryHyperCall::ReadGuestVirtualMemory(name, current_eprocess + g_ImageFileNameOffset, walk_cr3, 15) > 0) {
            bool is_match = false;
            if (pid != 0) {
                is_match = (current_pid == pid);
            } else {
                if (_stricmp(name, skCrypt("TslGame.exe")) == 0) {
                    is_match = true;
                }
            }

            if (is_match) {
                // [HEURISTIC V4] Stricter validation for the main game process
                // 1. Must have a valid CR3 (DirectoryTableBase)
                // 2. Must have a valid Base Address
                // 3. Must have a valid PEB
                // 4. Must have more than 10 threads (Main engine is heavily multi-threaded)
                if (current_cr3 != 0 && current_base != 0 && current_peb != 0 && current_threads > 10) {
                    if (pid != 0) {
                        best_pid = (std::uint32_t)current_pid;
                        best_cr3 = current_cr3;
                        best_base = current_base;
                        best_peb = current_peb;
                        break; 
                    } else {
                        if (best_pid == 0 || current_threads > max_threads) {
                            max_threads = current_threads;
                            best_pid = (std::uint32_t)current_pid;
                            best_cr3 = current_cr3;
                            best_base = current_base;
                            best_peb = current_peb;
                        }
                    }
                }
            }
        }

        std::uint64_t next_link = 0;
        if (telemetryHyperCall::ReadGuestVirtualMemory(&next_link, current_eprocess + eprocess::ActiveProcessLinks, walk_cr3, 8) != 8 || next_link == 0)
        {
            break;
        }
        current_eprocess = next_link - eprocess::ActiveProcessLinks;
    }
    while (current_eprocess != 0 && current_eprocess != start_eprocess);

    if (best_pid != 0) {
        output->process_id = best_pid;
        output->cr3 = best_cr3;
        output->base_address = reinterpret_cast<void*>(best_base);
        output->peb = reinterpret_cast<void*>(best_peb);
        return true;
    }

    return false;
}

std::vector<uint32_t> telemetryHyperProcess::FindAllPidsByGhostWalk(const char* target_name)
{
    std::vector<uint32_t> found_pids;
    if (!ResolvePsInitialSystemProcess()) return found_pids;

    std::uint64_t current_eprocess = 0;
    if (telemetryHyperCall::ReadGuestVirtualMemory(&current_eprocess, g_PsInitialSystemProcess, g_CurrentCr3, 8) != 8 || current_eprocess == 0) {
        return found_pids;
    }

    const auto start_eprocess = current_eprocess;
    int check_count = 0;
    do {
        std::uint64_t pid = 0;
        telemetryHyperCall::ReadGuestVirtualMemory(&pid, current_eprocess + eprocess::UniqueProcessId, g_CurrentCr3, 8);

        char name[16] = {};
        if (telemetryHyperCall::ReadGuestVirtualMemory(name, current_eprocess + g_ImageFileNameOffset, g_CurrentCr3, 15) > 0) {
            if (strstr(name, target_name) != nullptr) {
                found_pids.push_back((uint32_t)pid);
            }
        }

        std::uint64_t next_link = 0;
        telemetryHyperCall::ReadGuestVirtualMemory(&next_link, current_eprocess + eprocess::ActiveProcessLinks, g_CurrentCr3, 8);
        current_eprocess = next_link - eprocess::ActiveProcessLinks;
        check_count++;
    } while (current_eprocess != 0 && current_eprocess != start_eprocess && check_count < 2000);

    return found_pids;
}

DWORD telemetryHyperProcess::FindProcessIdByName(const wchar_t* const process_name)
{
    if (process_name == nullptr || process_name[0] == L'\0')
    {
        return 0;
    }

    if (!ResolvePsInitialSystemProcess()) return 0;

    std::uint64_t current_eprocess = 0;
    std::uint64_t walk_cr3 = g_CurrentCr3;

    if (telemetryHyperCall::ReadGuestVirtualMemory(&current_eprocess, g_PsInitialSystemProcess, walk_cr3, 8) != 8 || current_eprocess == 0)
    {
        return 0;
    }

    const auto start_eprocess = current_eprocess;
    do
    {
        std::uint64_t current_pid = 0;
        telemetryHyperCall::ReadGuestVirtualMemory(&current_pid, current_eprocess + eprocess::UniqueProcessId, walk_cr3, 8);

        char name[16] = {};
        if (telemetryHyperCall::ReadGuestVirtualMemory(name, current_eprocess + g_ImageFileNameOffset, walk_cr3, 15) > 0) {
            // Convert wchar_t input to char for comparison with kernel ImageFileName
            char target_name[16] = {};
            for (int i = 0; i < 15 && process_name[i] != L'\0'; ++i) {
                target_name[i] = (char)process_name[i];
            }

            if (_strnicmp(name, target_name, 15) == 0) {
                return (DWORD)current_pid;
            }
        }

        std::uint64_t next_link = 0;
        if (telemetryHyperCall::ReadGuestVirtualMemory(&next_link, current_eprocess + eprocess::ActiveProcessLinks, walk_cr3, 8) != 8 || next_link == 0)
        {
            break;
        }

        current_eprocess = next_link - eprocess::ActiveProcessLinks;
    }
    while (current_eprocess != 0 && current_eprocess != start_eprocess);

    return 0;
}
std::uint64_t telemetryHyperProcess::GetEProcessAddress(std::uint32_t pid)
{
    if (pid == 0 || !ResolvePsInitialSystemProcess()) return 0;

    std::uint64_t current_eprocess = 0;
    std::uint64_t walk_cr3 = g_CurrentCr3;

    if (telemetryHyperCall::ReadGuestVirtualMemory(&current_eprocess, g_PsInitialSystemProcess, walk_cr3, 8) != 8 || current_eprocess == 0)
        return 0;

    const auto start_eprocess = current_eprocess;
    do {
        std::uint64_t current_pid = 0;
        telemetryHyperCall::ReadGuestVirtualMemory(&current_pid, current_eprocess + eprocess::UniqueProcessId, walk_cr3, 8);

        if (current_pid == pid) {
            return current_eprocess;
        }

        std::uint64_t next_link = 0;
        if (telemetryHyperCall::ReadGuestVirtualMemory(&next_link, current_eprocess + eprocess::ActiveProcessLinks, walk_cr3, 8) != 8 || next_link == 0)
            break;

        current_eprocess = next_link - eprocess::ActiveProcessLinks;
    } while (current_eprocess != 0 && current_eprocess != start_eprocess);

    return 0;
}

bool telemetryHyperProcess::UnlinkProcessDKOM(std::uint64_t eprocess_address)
{
    if (eprocess_address == 0 || !ResolvePsInitialSystemProcess()) return false;

    std::uint64_t walk_cr3 = g_CurrentCr3;

    // ActiveProcessLinks is a LIST_ENTRY at offset 0x448
    // LIST_ENTRY { Flink (8 bytes), Blink (8 bytes) }
    std::uint64_t our_apl = eprocess_address + eprocess::ActiveProcessLinks;

    // Read our Flink and Blink
    std::uint64_t our_flink = 0, our_blink = 0;
    if (telemetryHyperCall::ReadGuestVirtualMemory(&our_flink, our_apl, walk_cr3, 8) != 8)
        return false;
    if (telemetryHyperCall::ReadGuestVirtualMemory(&our_blink, our_apl + 8, walk_cr3, 8) != 8)
        return false;

    if (our_flink == 0 || our_blink == 0) return false;

    // Flink points to NEXT->ActiveProcessLinks (which starts with Flink)
    // Blink points to PREV->ActiveProcessLinks (which starts with Flink, then Blink at +8)

    // Patch: PREV->Flink = our_flink (skip us)
    if (telemetryHyperCall::WriteGuestVirtualMemory(&our_flink, our_blink, walk_cr3, 8) != 8)
        return false;

    // Patch: NEXT->Blink = our_blink (skip us)
    if (telemetryHyperCall::WriteGuestVirtualMemory(&our_blink, our_flink + 8, walk_cr3, 8) != 8)
        return false;

    // Self-loop: point our own Flink/Blink to ourselves (safe for process exit)
    if (telemetryHyperCall::WriteGuestVirtualMemory(&our_apl, our_apl, walk_cr3, 8) != 8)
        return false;
    if (telemetryHyperCall::WriteGuestVirtualMemory(&our_apl, our_apl + 8, walk_cr3, 8) != 8)
        return false;

    return true;
}
