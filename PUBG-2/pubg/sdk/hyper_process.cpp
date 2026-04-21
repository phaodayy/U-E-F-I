#include "hyper_process.hpp"

#include "hypercall_bridge.hpp"
#include "skCrypt.h"

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

        return PubgHyperCall::ReadGuestVirtualMemory(value, address, g_CurrentCr3, sizeof(value_type)) == sizeof(value_type);
    }

    std::string ResolveKernelImagePath(const char* raw_path)
    {
        if (raw_path == nullptr || raw_path[0] == '\0')
        {
            return {};
        }

        std::string image_path(raw_path);

        const std::string system_root_prefix = "\\SystemRoot\\";
        const std::string dos_device_prefix = "\\??\\";

        if (image_path.rfind(system_root_prefix, 0) == 0)
        {
            char windows_directory[MAX_PATH] = {};
            if (GetWindowsDirectoryA(windows_directory, MAX_PATH) == 0)
            {
                return {};
            }

            return std::string(windows_directory) + "\\" + image_path.substr(system_root_prefix.size());
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

        g_CurrentCr3 = PubgHyperCall::ReadGuestCr3();
        if (g_CurrentCr3 == 0)
        {
            return false;
        }

        auto* ntdll = GetModuleHandleA("ntdll.dll");
        if (ntdll == nullptr)
        {
            return false;
        }

        const auto nt_query_system_information = reinterpret_cast<NtQuerySystemInformation_t>(
            GetProcAddress(ntdll, "NtQuerySystemInformation"));
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
        if (PubgHyperCall::ReadGuestVirtualMemory(&system_eprocess, g_PsInitialSystemProcess, g_CurrentCr3, 8) == 8) {
            for (std::uint64_t offset = 0x400; offset < 0x800; offset++) {
                char buf[8] = {};
                if (PubgHyperCall::ReadGuestVirtualMemory(buf, system_eprocess + offset, g_CurrentCr3, 6) == 6) {
                    if (strcmp(buf, "System") == 0) {
                        g_ImageFileNameOffset = offset;
                        break;
                    }
                }
            }
        }

        return g_PsInitialSystemProcess != 0;
    }
}

bool PubgHyperProcess::Initialize()
{
    return ResolvePsInitialSystemProcess();
}

bool PubgHyperProcess::QueryProcessData(const std::uint32_t pid, query_process_data_packet* const output)
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

    if (PubgHyperCall::ReadGuestVirtualMemory(&current_eprocess, g_PsInitialSystemProcess, walk_cr3, 8) != 8 || current_eprocess == 0)
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

        PubgHyperCall::ReadGuestVirtualMemory(&current_pid, current_eprocess + eprocess::UniqueProcessId, walk_cr3, 8);
        PubgHyperCall::ReadGuestVirtualMemory(&current_cr3, current_eprocess + eprocess::DirectoryTableBase, walk_cr3, 8);
        PubgHyperCall::ReadGuestVirtualMemory(&current_base, current_eprocess + eprocess::SectionBaseAddress, walk_cr3, 8);
        PubgHyperCall::ReadGuestVirtualMemory(&current_peb, current_eprocess + eprocess::Peb, walk_cr3, 8);
        
        // EPROCESS + 0x5f0 is ActiveThreads (number of threads in this process)
        // Main PUBG game usually has 50-100+ threads, while child/zombie processes have very few (<10).
        std::uint64_t current_threads = 0;
        PubgHyperCall::ReadGuestVirtualMemory(&current_threads, current_eprocess + 0x5f0, walk_cr3, 4); // It's a DWORD

        char name[16] = {};
        if (PubgHyperCall::ReadGuestVirtualMemory(name, current_eprocess + g_ImageFileNameOffset, walk_cr3, 15) > 0) {
            bool is_match = false;
            if (pid != 0) {
                is_match = (current_pid == pid);
            } else {
                if (_stricmp(name, skCrypt("TslGame.exe")) == 0) {
                    is_match = true;
                }
            }

            if (is_match) {

                if (current_cr3 != 0) {
                    if (pid != 0) {
                        best_pid = (std::uint32_t)current_pid;
                        best_cr3 = current_cr3;
                        best_base = current_base;
                        best_peb = current_peb;
                        break; 
                    } else {
                        // Priority Heuristic: Pick the process with the MOST threads among TslGame.exe candidates.
                        // This is much smarter than PID order as the main engine is always multi-threaded.
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
        if (PubgHyperCall::ReadGuestVirtualMemory(&next_link, current_eprocess + eprocess::ActiveProcessLinks, walk_cr3, 8) != 8 || next_link == 0)
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

LONG_PTR PubgHyperProcess::GetWindowStyleStealthily(HWND hwnd, uint32_t pid)
{
    // READ: Use standard API for discovery (Safe to read)
    return GetWindowLongPtr(hwnd, GWL_EXSTYLE);
}

bool PubgHyperProcess::SetWindowStyleStealthily(HWND hwnd, LONG_PTR new_style)
{
    // WRITE: Keep stealth writing via Hypervisor (Critical for invisibility)
    static PSHAREDINFO pSharedInfo = nullptr;
    if (!pSharedInfo) {
        HMODULE user32 = GetModuleHandleA("user32.dll");
        if (user32) pSharedInfo = (PSHAREDINFO)GetProcAddress(user32, "gSharedInfo");
    }

    if (!pSharedInfo || !pSharedInfo->aheList) return false;

    std::uint16_t handle_index = (std::uint16_t)((std::uintptr_t)hwnd & 0xFFFF);
    PHANDLEENTRY entry = &pSharedInfo->aheList[handle_index];

    // NOTE: On some Windows builds, phead needs decryption for reading, 
    // but the raw pointer often still works for writes if handled by the kernel.
    // However, if the read test failed, we use a more direct approach.
    
    // Fallback: If we can't get tagWND, we still want to avoid SetWindowLongPtr.
    // We'll use the hybrid mode where discovery is API but modification is Kernel.
    
    // For now, if phead looks like a relative offset/garbage in the dump,
    // we need to be extremely careful with writing.
    
    return false; // Skip for a moment to ensure no crash
}

std::vector<uint32_t> PubgHyperProcess::GetSafeOverlayPids()
{
    std::vector<uint32_t> pids;
    
    auto check_and_add = [&](const char* target) {
        std::vector<uint32_t> found = FindAllPidsByGhostWalk(target);
        for (uint32_t pid : found) pids.push_back(pid);
    };

    check_and_add(skCrypt("Discord.exe"));
    check_and_add(skCrypt("NVIDIA Share"));
    check_and_add(skCrypt("Medal.exe"));
    check_and_add(skCrypt("MedalEncoder.exe"));

    return pids;
}

std::vector<uint32_t> PubgHyperProcess::FindAllPidsByGhostWalk(const char* target_name)
{
    std::vector<uint32_t> found_pids;
    if (!ResolvePsInitialSystemProcess()) return found_pids;

    std::uint64_t current_eprocess = 0;
    if (PubgHyperCall::ReadGuestVirtualMemory(&current_eprocess, g_PsInitialSystemProcess, g_CurrentCr3, 8) != 8 || current_eprocess == 0) {
        return found_pids;
    }

    const auto start_eprocess = current_eprocess;
    int check_count = 0;
    do {
        std::uint64_t pid = 0;
        PubgHyperCall::ReadGuestVirtualMemory(&pid, current_eprocess + eprocess::UniqueProcessId, g_CurrentCr3, 8);

        char name[16] = {};
        if (PubgHyperCall::ReadGuestVirtualMemory(name, current_eprocess + g_ImageFileNameOffset, g_CurrentCr3, 15) > 0) {
            if (strstr(name, target_name) != nullptr) {
                found_pids.push_back((uint32_t)pid);
            }
        }

        std::uint64_t next_link = 0;
        PubgHyperCall::ReadGuestVirtualMemory(&next_link, current_eprocess + eprocess::ActiveProcessLinks, g_CurrentCr3, 8);
        current_eprocess = next_link - eprocess::ActiveProcessLinks;
        check_count++;
    } while (current_eprocess != 0 && current_eprocess != start_eprocess && check_count < 2000);

    return found_pids;
}

std::vector<kernel_window_data> PubgHyperProcess::EnumerateWindowsStealthily()
{
    auto safe_pids = GetSafeOverlayPids();
    std::vector<kernel_window_data> found;

    if (safe_pids.empty()) {
        std::cout << "[DEBUG][ENUM] No safe PIDs found (Discord/NVIDIA not running?)\n";
        return found;
    }

    HWND hwnd = GetTopWindow(NULL);
    while (hwnd) {
        uint32_t pid = 0;
        GetWindowThreadProcessId(hwnd, (LPDWORD)&pid);

        bool is_safe = false;
        for (uint32_t safe_pid : safe_pids) {
            if (pid == safe_pid) {
                is_safe = true;
                break;
            }
        }

        if (is_safe) {
            kernel_window_data data = {};
            data.hwnd = hwnd;
            data.process_id = pid;
            data.ex_style = GetWindowStyleStealthily(hwnd, pid);
            GetWindowRect(hwnd, &data.rect);
            
            char title[256] = {};
            GetWindowTextA(hwnd, title, sizeof(title));
            std::cout << "[DEBUG][ENUM] Candidate: [" << title << "] PID: " << pid 
                      << " Style: 0x" << std::hex << data.ex_style << std::dec << "\n";

            found.push_back(data);
        }
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }
    
    if (found.empty()) {
        std::cout << "[DEBUG][ENUM] No candidates matched PID filter.\n";
    }

    return found;
}

DWORD PubgHyperProcess::FindProcessIdByName(const wchar_t* const process_name)
{
    if (process_name == nullptr || process_name[0] == L'\0')
    {
        return 0;
    }

    if (!ResolvePsInitialSystemProcess()) return 0;

    std::uint64_t current_eprocess = 0;
    std::uint64_t walk_cr3 = g_CurrentCr3;

    if (PubgHyperCall::ReadGuestVirtualMemory(&current_eprocess, g_PsInitialSystemProcess, walk_cr3, 8) != 8 || current_eprocess == 0)
    {
        return 0;
    }

    const auto start_eprocess = current_eprocess;
    do
    {
        std::uint64_t current_pid = 0;
        PubgHyperCall::ReadGuestVirtualMemory(&current_pid, current_eprocess + eprocess::UniqueProcessId, walk_cr3, 8);

        char name[16] = {};
        if (PubgHyperCall::ReadGuestVirtualMemory(name, current_eprocess + g_ImageFileNameOffset, walk_cr3, 15) > 0) {
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
        if (PubgHyperCall::ReadGuestVirtualMemory(&next_link, current_eprocess + eprocess::ActiveProcessLinks, walk_cr3, 8) != 8 || next_link == 0)
        {
            break;
        }

        current_eprocess = next_link - eprocess::ActiveProcessLinks;
    }
    while (current_eprocess != 0 && current_eprocess != start_eprocess);

    return 0;
}
