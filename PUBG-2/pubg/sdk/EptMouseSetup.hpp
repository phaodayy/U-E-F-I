#pragma once
#include <windows.h>
#include <winternl.h>
#include <string>
#include <vector>
#include <iostream>
#include "scanner.hpp"
#include "memory.hpp"
#include "hypercall_bridge.hpp"

#pragma comment(lib, "ntdll.lib")

typedef struct _RTL_PROCESS_MODULE_INFORMATION {
    HANDLE Section;
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR FullFullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

constexpr ULONG SystemModuleInformation = 11;

namespace EptMouse {

    inline uint64_t GetKernelModuleBase(const char* moduleName, ULONG* outSize) {
        ULONG returnLength = 0;
        NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)SystemModuleInformation, NULL, 0, &returnLength);
        if (returnLength == 0) return 0;
        
        std::vector<uint8_t> buffer(returnLength);
        NTSTATUS status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)SystemModuleInformation, buffer.data(), returnLength, &returnLength);
        if (!NT_SUCCESS(status)) return 0;
        
        RTL_PROCESS_MODULES* modules = (RTL_PROCESS_MODULES*)buffer.data();
        for (ULONG i = 0; i < modules->NumberOfModules; i++) {
            const char* fullPath = (const char*)modules->Modules[i].FullFullPathName;
            const char* name = fullPath + modules->Modules[i].OffsetToFileName;
            
            if (_stricmp(name, moduleName) == 0) {
                if (outSize) *outSize = modules->Modules[i].ImageSize;
                return (uint64_t)modules->Modules[i].ImageBase;
            }
        }
        return 0;
    }

    // Helper reader logic to map Hypercall memory reading
    inline bool ReadKernelMemory(uint64_t address, void* buffer, size_t size) {
        // Find SYSTEM process CR3
        static uint64_t systemCr3 = 0;
        if (systemCr3 == 0) {
            // Usually we can just ask Hypervisor to read from cr3=0 (host physical) or System CR3.
            // But we can use the caller's CR3 if it's mapped in kernel space! Kernel space is mapped in all CR3s (KVA Shadow might slightly affect it but Drivers usually accessible).
            systemCr3 = PubgMemory::g_ProcessCr3; // Or any valid CR3
        }
        return PubgHyperCall::ReadGuestVirtualMemory(buffer, address, systemCr3, size) == size;
    }

    inline uint64_t InstallMouseEptHook() {
        ULONG size = 0;
        uint64_t mouclassBase = GetKernelModuleBase("mouclass.sys", &size);
        if (!mouclassBase) return 0;

        // List of known byte patterns for MouseClassServiceCallback in Win10/Win11
        const char* patterns[] = {
            "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 83 EC",
            "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC",
            "48 8B C4 55 56 57 41 54 41 55 41 56 41 57 48 8D 68 ? 48 81 EC"
        };

        for (const char* ptn : patterns) {
            uint64_t foundAddr = Scanner::PatternScan(
                mouclassBase, size, ptn, 
                [](uint64_t addr, void* buf, size_t sz) {
                    return ReadKernelMemory(addr, buf, sz);
                }
            );

            if (foundAddr) {
                // Address found! Send it to Ring -1 Hypervisor to commit the EPT hook.
                hypercall_info_t info = {};
                // The actual keys would be populated by launch_raw_hypercall automatically based on current_primary_key.
                // However, we need to add a wrapper in hypercall_bridge.hpp
                // PubgHyperCall::SetMouseHookAddress(foundAddr);
                return foundAddr;
            }
        }
        return 0;
    }
}
