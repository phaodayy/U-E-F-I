#pragma once
#include <cstdint>
#include <iostream>
#include <.shared/shared.hpp>
#include "memory.hpp"

namespace telemetryDecrypt {
    enum class OpType { OP_XOR, OP_ADD, OP_SUB, OP_ROL_64, OP_ROL_8 };
    
    struct Step {
        OpType Type;
        uint64_t Val;
    };

    inline std::vector<Step> g_Steps;
    inline uint64_t g_RaxKey = 0;

    template<typename T>
    inline bool Initialize(T read_func, uint64_t base_address, uint64_t xenuine_decrypt_offset) {
        g_Steps.clear();
        g_RaxKey = 0;

        uint64_t funcAddr = 0;
        uintptr_t targetAddr = base_address + xenuine_decrypt_offset;

        // Read the pointer to the decryption function
        if (!read_func(targetAddr, &funcAddr, sizeof(uint64_t)) || funcAddr < 0x1000000) {
            return false;
        }

        uint8_t bytes[128] = { 0 };
        if (!read_func(funcAddr, bytes, sizeof(bytes))) return false;

        // 1. Extract RAX Key (usually a LEA or MOV instruction at the start)
        if (bytes[0] == 0x48 && bytes[2] == 0x05) {
            int32_t delta = *reinterpret_cast<int32_t*>(&bytes[3]);
            uint64_t addr = funcAddr + 7 + delta;
            
            if (bytes[1] == 0x8D) { // LEA RAX, [RIP + delta]
                g_RaxKey = addr;
            } else if (bytes[1] == 0x8B) { // MOV RAX, [RIP + delta]
                if (!read_func(addr, &g_RaxKey, sizeof(uint64_t))) return false;
            }
        }

        // 2. Parse Bytecode for math operations
        for (size_t i = 0; i < sizeof(bytes) - 6; i++) {
            if (bytes[i] == 0xC3) break; // RET instruction

            // XOR RCX, imm32 (48 81 F1 [imm32])
            if (bytes[i] == 0x48 && bytes[i + 1] == 0x81 && bytes[i + 2] == 0xF1) {
                g_Steps.push_back({ OpType::OP_XOR, *reinterpret_cast<uint32_t*>(&bytes[i + 3]) });
                i += 6;
            }
            // XOR RCX, RAX (48 31 C1)
            else if (bytes[i] == 0x48 && bytes[i + 1] == 0x31 && bytes[i + 2] == 0xC1) {
                g_Steps.push_back({ OpType::OP_XOR, g_RaxKey });
                i += 2;
            }
            // ADD RCX, imm32 (48 81 C1 [imm32])
            else if (bytes[i] == 0x48 && bytes[i + 1] == 0x81 && bytes[i + 2] == 0xC1) {
                g_Steps.push_back({ OpType::OP_ADD, *reinterpret_cast<uint32_t*>(&bytes[i + 3]) });
                i += 6;
            }
            // ADD RCX, RAX (48 01 C1)
            else if (bytes[i] == 0x48 && bytes[i + 1] == 0x01 && bytes[i + 2] == 0xC1) {
                g_Steps.push_back({ OpType::OP_ADD, g_RaxKey });
                i += 2;
            }
            // SUB RCX, imm32 (48 81 E9 [imm32])
            else if (bytes[i] == 0x48 && bytes[i + 1] == 0x81 && bytes[i + 2] == 0xE9) {
                g_Steps.push_back({ OpType::OP_SUB, *reinterpret_cast<uint32_t*>(&bytes[i + 3]) });
                i += 6;
            }
            // SUB RCX, RAX (48 29 C1)
            else if (bytes[i] == 0x48 && bytes[i + 1] == 0x29 && bytes[i + 2] == 0xC1) {
                g_Steps.push_back({ OpType::OP_SUB, g_RaxKey });
                i += 2;
            }
            // ROL RCX, imm8 (48 C1 C1 [imm8])
            else if (bytes[i] == 0x48 && bytes[i + 1] == 0xC1 && bytes[i + 2] == 0xC1) {
                g_Steps.push_back({ OpType::OP_ROL_64, bytes[i + 3] });
                i += 3;
            }
            // ROL CL, imm8 (C0 C1 [imm8])
            else if (bytes[i] == 0xC0 && bytes[i + 1] == 0xC1) {
                g_Steps.push_back({ OpType::OP_ROL_8, bytes[i + 2] });
                i += 2;
            }
        }

#ifdef _DEBUG
        if (!g_Steps.empty()) {
            std::cout << skCrypt("[DECRYPT] Emulator ready with ") << g_Steps.size() << skCrypt(" steps. Key: ") << std::hex << g_RaxKey << std::dec << std::endl;
        }
#endif
        return !g_Steps.empty();
    }

    inline uint64_t Xe(uint64_t addr) {
        if (addr == 0 || g_Steps.empty()) return 0;
        
        uint64_t r = addr;
        for (const auto& op : g_Steps) {
            switch (op.Type) {
            case OpType::OP_XOR: r ^= op.Val; break;
            case OpType::OP_ADD: r += op.Val; break;
            case OpType::OP_SUB: r -= op.Val; break;
            case OpType::OP_ROL_64: {
                int s = static_cast<int>(op.Val & 63);
                r = (r << s) | (r >> (64 - s));
                break;
            }
            case OpType::OP_ROL_8: {
                uint8_t c = static_cast<uint8_t>(r & 0xFF);
                int a = static_cast<int>(op.Val & 7);
                c = (c << a) | (c >> (8 - a));
                r = (r & 0xFFFFFFFFFFFFFF00) | c;
                break;
            }
            }
        }
        // Cleanup: Mask to 48-bit canonical address (removes 0x1000... prefixes)
        return r & 0x0000FFFFFFFFFFFFULL;
    }

    inline void Cleanup() {
        g_Steps.clear();
        g_RaxKey = 0;
    }
}
