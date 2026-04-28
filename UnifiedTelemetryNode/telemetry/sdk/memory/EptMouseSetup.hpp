#pragma once
#include <windows.h>
#include <winternl.h>
#include <string>
#include <vector>
#include <iostream>
#include "scanner.hpp"
#include "memory.hpp"
#include "hypercall_bridge.hpp"
#include "hyper_process.hpp"

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

    inline bool ReadKernelMemory(uint64_t address, void* buffer, size_t size) {
        static uint64_t systemCr3 = 0;
        if (systemCr3 == 0) {
            query_process_data_packet system_data = {};
            if (telemetryHyperProcess::QueryProcessData(4, &system_data)) {
                systemCr3 = system_data.cr3;
            }
        }
        uint64_t cr3 = (systemCr3 != 0) ? systemCr3 : telemetryMemory::g_ProcessCr3;
        if (cr3 == 0) return false;
        return telemetryHyperCall::ReadGuestVirtualMemory(buffer, address, cr3, size) == size;
    }

    inline uint64_t InstallMouseEptHook() {
        uint64_t mouclassBase = telemetryHyperProcess::GetKernelModuleBase(skCrypt("mouclass.sys"));
        if (!mouclassBase) return 0;

        // 1. Resolve .text section via PE Header analysis
        IMAGE_DOS_HEADER dosHeader;
        if (!ReadKernelMemory(mouclassBase, &dosHeader, sizeof(dosHeader)) || dosHeader.e_magic != 0x5A4D) return 0;

        IMAGE_NT_HEADERS64 ntHeaders;
        if (!ReadKernelMemory(mouclassBase + dosHeader.e_lfanew, &ntHeaders, sizeof(ntHeaders))) return 0;

        uint64_t sectionHeaderAddr = mouclassBase + dosHeader.e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + ntHeaders.FileHeader.SizeOfOptionalHeader;
        
        uint32_t textRva = 0;
        uint32_t textSize = 0;

        for (int i = 0; i < ntHeaders.FileHeader.NumberOfSections; i++) {
            IMAGE_SECTION_HEADER section;
            if (ReadKernelMemory(sectionHeaderAddr + (i * sizeof(IMAGE_SECTION_HEADER)), &section, sizeof(section))) {
                if (strncmp((const char*)section.Name, skCrypt(".text"), 5) == 0) {
                    textRva = section.VirtualAddress;
                    textSize = section.Misc.VirtualSize;
                    break;
                }
            }
        }

#ifdef _DEBUG
        std::cout << skCrypt("[DEBUG] mouclass.sys .text RVA: 0x") << std::hex << textRva << skCrypt(" size: 0x") << textSize << std::dec << std::endl;
#endif

        if (textRva == 0) textRva = 0x1000; // Fallback
        if (textSize == 0) textSize = 0x50000;

        // 2. Try RVAs from User Snippet again with verified memory
        uint64_t candidateRVAs[] = { 0x18A0, 0x18B0, 0x1810, 0x1920 }; 
        for (uint64_t rva : candidateRVAs) {
            uint64_t addr = mouclassBase + rva;
            uint8_t prologue[4] = {0};
            if (ReadKernelMemory(addr, prologue, 4)) {
                if (prologue[0] == 0x48 || prologue[0] == 0x40 || prologue[0] == 0x44) {
#ifdef _DEBUG
                    std::cout << skCrypt("[+] Safe Mode: Skipping EPT Hook on 0x") << std::hex << addr << std::dec << std::endl;
#endif
                    // [TEST] Temporarily disabled to check if 0x220 works alone
                    // if (telemetryHyperCall::SetMouseHookAddress(addr)) return addr;
                    return addr; // Pretend it's hooked
                }
            }
        }

        // 3. Scan the actual .text section
        const char* patterns[] = {
            skCrypt("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 83 EC"),
            skCrypt("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC"),
            skCrypt("48 8B C4 ? ? ? ? ? ? ? ? ? ? ? ? 57 41 54 41 55 41 56 41 57 48 83 EC"),
            skCrypt("40 53 56 57 41 54 41 55 41 56 41 57 48 83 EC 60")
        };

        for (const char* ptn : patterns) {
            uint64_t foundAddr = Scanner::PatternScan(
                mouclassBase + textRva, textSize, ptn, 
                [](uint64_t addr, void* buf, size_t sz) {
                    return ReadKernelMemory(addr, buf, sz);
                }
            );

            if (foundAddr) {
#ifdef _DEBUG
                std::cout << skCrypt("[DEBUG] MouseCallback found at .text+0x") << std::hex << (foundAddr - (mouclassBase + textRva)) << std::dec << std::endl;
#endif
                if (telemetryHyperCall::SetMouseHookAddress(foundAddr)) return foundAddr;
            }
        }

#ifdef _DEBUG
        std::cout << skCrypt("[!] FAILED: No mouse callback found in .text section.") << std::endl;
#endif
        return 0;
    }
}
