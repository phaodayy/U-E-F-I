#include "hyper_process.hpp"

#include "hypercall_bridge.hpp"

#include <TlHelp32.h>
#include <winternl.h>

#include <memory>
#include <string>
#include <winuser.h>

typedef struct _HANDLEENTRY {
    PVOID  phead;
    PVOID  pOwner;
    BYTE   bType;
    BYTE   bFlags;
    WORD   wUnused;
} HANDLEENTRY, * PHANDLEENTRY;

typedef struct _SERVERINFO {
    DWORD dwSRVIFlags;
    std::uint64_t cHandleEntries;
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

        const auto export_address = reinterpret_cast<std::uint8_t*>(GetProcAddress(local_ntoskrnl, "PsInitialSystemProcess"));
        if (export_address == nullptr)
        {
            FreeLibrary(local_ntoskrnl);
            return false;
        }

        const auto export_rva = reinterpret_cast<std::uintptr_t>(export_address) - reinterpret_cast<std::uintptr_t>(local_ntoskrnl);
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

    do
    {
        std::uint64_t current_pid = 0;
        std::uint64_t current_cr3 = 0;
        std::uint64_t current_base = 0;
        std::uint64_t current_peb = 0;

        PubgHyperCall::ReadGuestVirtualMemory(&current_pid, current_eprocess + eprocess::UniqueProcessId, walk_cr3, 8);
        PubgHyperCall::ReadGuestVirtualMemory(&current_cr3, current_eprocess + eprocess::DirectoryTableBase, walk_cr3, 8);
        PubgHyperCall::ReadGuestVirtualMemory(&current_base, current_eprocess + eprocess::SectionBaseAddress, walk_cr3, 8);
        PubgHyperCall::ReadGuestVirtualMemory(&current_peb, current_eprocess + eprocess::Peb, walk_cr3, 8);

        bool match = false;
        char name[16] = {};
        if (PubgHyperCall::ReadGuestVirtualMemory(name, current_eprocess + g_ImageFileNameOffset, walk_cr3, 15) > 0) {
            if (pid != 0) {
                match = (current_pid == pid);
            } else {
                if (_strnicmp(name, "TslGame", 7) == 0) {
                    match = true;
                }
            }
        }

        if (match && current_cr3 != 0)
        {
            output->process_id = static_cast<uint32_t>(current_pid);
            output->cr3 = current_cr3;
            output->base_address = reinterpret_cast<void*>(current_base);
            output->peb = reinterpret_cast<void*>(current_peb);
            return true;
        }

        std::uint64_t next_link = 0;
        if (PubgHyperCall::ReadGuestVirtualMemory(&next_link, current_eprocess + eprocess::ActiveProcessLinks, walk_cr3, 8) != 8 || next_link == 0)
        {
            break;
        }

        current_eprocess = next_link - eprocess::ActiveProcessLinks;
    }
    while (current_eprocess != 0 && current_eprocess != start_eprocess);

    return false;
}

bool PubgHyperProcess::SetWindowStyleStealthily(HWND hwnd, LONG_PTR new_style)
{
    static PSHAREDINFO pSharedInfo = nullptr;
    if (!pSharedInfo) {
        HMODULE user32 = GetModuleHandleA("user32.dll");
        if (user32) pSharedInfo = (PSHAREDINFO)GetProcAddress(user32, "gSharedInfo");
    }

    if (!pSharedInfo || !pSharedInfo->aheList) return false;

    // HWND is basically an index: (HANDLE & 0xFFFF)
    std::uint16_t handle_index = (std::uint16_t)((std::uintptr_t)hwnd & 0xFFFF);
    PHANDLEENTRY entry = &pSharedInfo->aheList[handle_index];

    // phead points to tagWND in kernel memory
    std::uint64_t tagWND_ptr = (std::uint64_t)entry->phead;
    if (!tagWND_ptr) return false;

    // tagWND->ExStyle offset is usually 0x18 on Win10/11 x64
    // We update it directly via hypercall (WriteGuestVirtualMemory uses System CR3 normally or current if it maps it)
    std::uint64_t ex_style_addr = tagWND_ptr + 0x18;
    
    // We use the launcher's CR3 since we are writing to a shared kernel area or a mapped area
    // Actually, writing to tagWND usually requires the session's win32k CR3, 
    // but the hypervisor can write to ANY kernel address if it translates correctly.
    // We'll use g_CurrentCr3 (Launcher) which we know can read/write kernel memory.
    
    return PubgHyperCall::WriteGuestVirtualMemory(&new_style, ex_style_addr, g_CurrentCr3, sizeof(new_style)) == sizeof(new_style);
}

DWORD PubgHyperProcess::FindProcessIdByName(const wchar_t* const process_name)
{
    if (process_name == nullptr || process_name[0] == L'\0')
    {
        return 0;
    }

    const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    PROCESSENTRY32W process_entry = {};
    process_entry.dwSize = sizeof(process_entry);

    DWORD result = 0;

    if (Process32FirstW(snapshot, &process_entry))
    {
        do
        {
            if (_wcsicmp(process_entry.szExeFile, process_name) == 0)
            {
                result = process_entry.th32ProcessID;
                break;
            }
        }
        while (Process32NextW(snapshot, &process_entry));
    }

    CloseHandle(snapshot);
    return result;
}
