#pragma once
#include <type_traits>

namespace skc
{
	template <int _size, char _key1, char _key2, typename T>
	class skCrypter
	{
	public:
		__forceinline constexpr skCrypter(T* data)
		{
			for (int i = 0; i < _size; i++)
			{
				_storage[i] = data[i] ^ (_key1 + i % (1 + _key2));
			}
		}

		__forceinline T* decrypt()
		{
			for (int i = 0; i < _size; i++)
			{
				_storage[i] = _storage[i] ^ (_key1 + i % (1 + _key2));
			}
			return _storage;
		}

		__forceinline operator T* ()
		{
			return decrypt();
		}

	private:
		T _storage[_size]{};
	};
}

#define skCrypt(str) []() { \
			constexpr static auto crypted = skc::skCrypter \
				<sizeof(str) / sizeof(str[0]), __TIME__[4], __TIME__[7], std::remove_const_t<std::remove_reference_t<decltype(str[0])>>>((std::remove_const_t<std::remove_reference_t<decltype(str[0])>>*)str); \
					return crypted; }().decrypt()
