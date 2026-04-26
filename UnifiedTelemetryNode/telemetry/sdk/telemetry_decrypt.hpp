#pragma once
#include <cstdint>
#include <iostream>
#include <.shared/shared.hpp>
#include "memory.hpp"

namespace telemetryDecrypt {
    inline uint64_t g_Ptr = 0;
    typedef uint64_t(*DecFunction_t)(uint64_t key, uint64_t addr);
    inline DecFunction_t g_DecFunction = nullptr;

    template<typename T>
    inline bool Initialize(T read_func, uint64_t base_address, uint64_t xenuine_decrypt_offset) {
        uint64_t decryptPtr = 0;
        uintptr_t targetAddr = base_address + xenuine_decrypt_offset;

        // Doc con tro decrypt tu Game
        if (!read_func(targetAddr, &decryptPtr, sizeof(uint64_t)) || decryptPtr < 0x10000) {
            decryptPtr = targetAddr;
        }

        int32_t tmp1Add = 0;
        if (!read_func(decryptPtr + 3, &tmp1Add, sizeof(int32_t))) return false;

        g_Ptr = tmp1Add + decryptPtr + 7;

        uint8_t shellcode[1024] = { 0 };
        
        // Doc Shellcode tu Game
        if (!read_func(decryptPtr, &shellcode[2], sizeof(shellcode) - 2)) return false;

        // Fix logic shellcode
        shellcode[0] = 0x90; shellcode[1] = 0x90;
        shellcode[2] = 0x48; shellcode[3] = 0x8B; shellcode[4] = 0xC1;
        shellcode[5] = 0x90; shellcode[6] = 0x90; shellcode[7] = 0x90; shellcode[8] = 0x90;

        // USERNODE DECRYPTION (SKIP DRIVER)
        if (g_DecFunction) VirtualFree((void*)g_DecFunction, 0, MEM_RELEASE);
        
        g_DecFunction = (DecFunction_t)VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!g_DecFunction) return false;

        memcpy((void*)g_DecFunction, shellcode, sizeof(shellcode));
        DWORD oldProt;
        VirtualProtect((void*)g_DecFunction, 4096, PAGE_EXECUTE_READ, &oldProt);

        return true;
    }

    inline uint64_t Xe(uint64_t addr) {
        if (!addr || !g_DecFunction) return 0;
        try {
            return g_DecFunction(g_Ptr, addr);
        } catch (...) {
            return 0;
        }
    }

    inline void Cleanup() {
        if (g_DecFunction) {
            VirtualFree((void*)g_DecFunction, 0, MEM_RELEASE);
            g_DecFunction = nullptr;
        }
    }
}
