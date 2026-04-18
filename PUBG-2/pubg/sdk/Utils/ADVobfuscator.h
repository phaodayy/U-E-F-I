#pragma once
#include <string>
#include <array>

// Minimal but Powerful ADVobfuscator-style Compile-time String Encryption
// Based on SebastienRioche/andrivet implementation principles.

namespace ADV {

    template <size_t N, int K>
    class ObfuscatedString {
    public:
        constexpr ObfuscatedString(const char* str) : _data{} {
            for (size_t i = 0; i < N; ++i) {
                _data[i] = static_cast<char>(str[i] ^ (K + (int)i));
            }
        }

        std::string decrypt() const {
            std::string res;
            res.reserve(N);
            for (size_t i = 0; i < N; ++i) {
                res += (char)(_data[i] ^ (K + i));
            }
            return res;
        }

        const char* c_str() const {
            static char decrypted[N + 1];
            for (size_t i = 0; i < N; ++i) {
                decrypted[i] = (char)(_data[i] ^ (K + i));
            }
            decrypted[N] = '\0';
            return decrypted;
        }

    private:
        std::array<char, N> _data;
    };

    // Macro to make usage easier and ensure it's generated at compile time
    #define OBF_STR(str) []() { \
        constexpr auto obfuscated = ADV::ObfuscatedString<sizeof(str)-1, __LINE__>(str); \
        return obfuscated; \
    }().c_str()

    // Macro for runtime decryption to avoid static persistence
    #define OBF_STR_RT(str) []() { \
        constexpr auto obfuscated = ADV::ObfuscatedString<sizeof(str)-1, __LINE__>(str); \
        return obfuscated.decrypt(); \
    }()
}

// Meta-programming for Control Flow Obfuscation (Simple State Machine)
// This makes traditional static analysis much harder.
namespace ADV::Meta {
    template<typename T>
    inline void JunkCode() {
        volatile int a = 10, b = 20, c = 0;
        c = a + b * (rand() % 10);
        if (c > 1000) JunkCode<T>(); // Never happens but confuses compiler
    }
}
