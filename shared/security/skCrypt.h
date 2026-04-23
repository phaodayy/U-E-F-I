/*
 * skCrypt
 * Faster & Better string encryption than ADVobfuscator
 * https://github.com/skadro-official/skCrypt
 */

#pragma once

#ifdef _DEBUG
    #define skCrypt(str) str
#else
    #include <string>
    #include <array>
    #include <cstdint>

    namespace skc
    {
        template <std::size_t size, std::uint8_t key, typename T>
        class skCrypt_t
        {
        public:
            __forceinline constexpr skCrypt_t(T* data)
            {
                for (std::size_t i = 0; i < size; i++)
                {
                    _data[i] = data[i] ^ (key + i);
                }
            }

            __forceinline T* decrypt()
            {
                for (std::size_t i = 0; i < size; i++)
                {
                    _data[i] = _data[i] ^ (key + i);
                }
                return _data.data();
            }

            __forceinline T* get()
            {
                return _data.data();
            }

        private:
            std::array<T, size> _data;
        };
    }

    #define skCrypt(str) []() { \
        constexpr std::uint8_t key = __TIME__[7]; \
        static auto crypt = skc::skCrypt_t<sizeof(str) / sizeof(str[0]), key, std::remove_const_t<std::remove_reference_t<decltype(str[0])>>>((std::remove_const_t<std::remove_reference_t<decltype(str[0])>>*)str); \
        return crypt.decrypt(); \
    }()
#endif
