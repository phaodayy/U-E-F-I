#pragma once
#include <Windows.h>
#include <winternl.h>
#include <vector>
#include <iostream>

/**
 * @brief Tối giản hóa PE Loader để nạp DLL trực tiếp từ RAM
 * Phù hợp để nạp tàng hình các Driver Usermode (Logitech, KMBox...) không muốn lộ file ra đĩa.
 */
namespace MemoryDllLoader {

    struct EXPORT_DIRECTORY {
        DWORD Characteristics;
        DWORD TimeDateStamp;
        WORD MajorVersion;
        WORD MinorVersion;
        DWORD Name;
        DWORD Base;
        DWORD NumberOfFunctions;
        DWORD NumberOfNames;
        DWORD AddressOfFunctions;
        DWORD AddressOfNames;
        DWORD AddressOfNameOrdinals;
    };

    inline void* MapDll(const void* pData) {
        if (!pData) return nullptr;

        const auto* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(pData);
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;

        const auto* ntHeader = reinterpret_cast<const IMAGE_NT_HEADERS*>((const char*)pData + dosHeader->e_lfanew);
        if (ntHeader->Signature != IMAGE_NT_SIGNATURE) return nullptr;

        if (ntHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            std::cout << "[!] Chỉ hỗ trợ nạp DLL x64 tàng hình." << std::endl;
            return nullptr;
        }

        // 1. Phân bổ bộ nhớ cho toàn bộ DLL (ImageSize)
        void* imageBase = VirtualAlloc(NULL, ntHeader->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!imageBase) return nullptr;

        // 2. Chép Headers
        memcpy(imageBase, pData, ntHeader->OptionalHeader.SizeOfHeaders);

        // 3. Chép Sections
        auto* sectionHeader = IMAGE_FIRST_SECTION(ntHeader);
        for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++, sectionHeader++) {
            if (sectionHeader->SizeOfRawData > 0) {
                memcpy((char*)imageBase + sectionHeader->VirtualAddress, (const char*)pData + sectionHeader->PointerToRawData, sectionHeader->SizeOfRawData);
            }
        }

        // 4. Giải quyết Base Relocation (Khớp địa chỉ nạp)
        auto& relocDir = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
        if (relocDir.Size > 0) {
            auto delta = (uintptr_t)imageBase - ntHeader->OptionalHeader.ImageBase;
            auto* relocBase = (IMAGE_BASE_RELOCATION*)((char*)imageBase + relocDir.VirtualAddress);

            while (relocBase->VirtualAddress != 0) {
                int count = (relocBase->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
                WORD* list = (WORD*)(relocBase + 1);

                for (int i = 0; i < count; i++) {
                    if (list[i] >> 12 == IMAGE_REL_BASED_DIR64) {
                        uintptr_t* p = (uintptr_t*)((char*)imageBase + relocBase->VirtualAddress + (list[i] & 0xFFF));
                        *p += delta;
                    }
                }
                relocBase = (IMAGE_BASE_RELOCATION*)((char*)relocBase + relocBase->SizeOfBlock);
            }
        }

        // 5. Giải quyết Import (Nạp các hàm API hệ thống)
        auto& importDir = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
        if (importDir.Size > 0) {
            auto* importDesc = (IMAGE_IMPORT_DESCRIPTOR*)((char*)imageBase + importDir.VirtualAddress);

            while (importDesc->Name != 0) {
                const char* libName = (const char*)imageBase + importDesc->Name;
                HMODULE hLib = LoadLibraryA(libName);
                if (hLib) {
                    uintptr_t* thunk = (uintptr_t*)((char*)imageBase + importDesc->FirstThunk);
                    uintptr_t* originalThunk = (uintptr_t*)((char*)imageBase + (importDesc->OriginalFirstThunk ? importDesc->OriginalFirstThunk : importDesc->FirstThunk));

                    while (*originalThunk != 0) {
                        if (IMAGE_SNAP_BY_ORDINAL(*originalThunk)) {
                            *thunk = (uintptr_t)GetProcAddress(hLib, (const char*)(*originalThunk & 0xFFFF));
                        } else {
                            auto* importByName = (IMAGE_IMPORT_BY_NAME*)((char*)imageBase + (*originalThunk));
                            *thunk = (uintptr_t)GetProcAddress(hLib, importByName->Name);
                        }
                        thunk++;
                        originalThunk++;
                    }
                }
                importDesc++;
            }
        }

        // 6. Gọi DllMain (Khởi tạo DLL nội bộ)
        if (ntHeader->OptionalHeader.AddressOfEntryPoint != 0) {
            using f_DllMain = BOOL(WINAPI*)(HINSTANCE, DWORD, LPVOID);
            auto DllMain = (f_DllMain)((char*)imageBase + ntHeader->OptionalHeader.AddressOfEntryPoint);
            DllMain((HINSTANCE)imageBase, DLL_PROCESS_ATTACH, NULL);
        }

        return imageBase;
    }

    inline void* GetExport(void* imageBase, const char* name) {
        const auto* dosHeader = (IMAGE_DOS_HEADER*)imageBase;
        const auto* ntHeader = (IMAGE_NT_HEADERS*)((char*)imageBase + dosHeader->e_lfanew);
        auto& exportDirAttr = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        if (exportDirAttr.Size == 0) return nullptr;

        auto* exportDir = (EXPORT_DIRECTORY*)((char*)imageBase + exportDirAttr.VirtualAddress);
        DWORD* funcTable = (DWORD*)((char*)imageBase + exportDir->AddressOfFunctions);
        DWORD* nameTable = (DWORD*)((char*)imageBase + exportDir->AddressOfNames);
        WORD* ordinalTable = (WORD*)((char*)imageBase + exportDir->AddressOfNameOrdinals);

        for (DWORD i = 0; i < exportDir->NumberOfNames; i++) {
            const char* expName = (const char*)imageBase + nameTable[i];
            if (strcmp(expName, name) == 0) {
                return (char*)imageBase + funcTable[ordinalTable[i]];
            }
        }
        return nullptr;
    }

    inline void FreeDll(void* imageBase) {
        if (!imageBase) return;
        const auto* dosHeader = (IMAGE_DOS_HEADER*)imageBase;
        const auto* ntHeader = (IMAGE_NT_HEADERS*)((char*)imageBase + dosHeader->e_lfanew);

        if (ntHeader->OptionalHeader.AddressOfEntryPoint != 0) {
            using f_DllMain = BOOL(WINAPI*)(HINSTANCE, DWORD, LPVOID);
            auto DllMain = (f_DllMain)((char*)imageBase + ntHeader->OptionalHeader.AddressOfEntryPoint);
            DllMain((HINSTANCE)imageBase, DLL_PROCESS_DETACH, NULL);
        }

        VirtualFree(imageBase, 0, MEM_RELEASE);
    }
}
