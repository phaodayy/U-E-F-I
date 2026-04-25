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
			if (_decrypted) return _storage;

			for (int i = 0; i < _size; i++)
			{
				_storage[i] = _storage[i] ^ (_key1 + i % (1 + _key2));
			}

			_decrypted = true;
			return _storage;
		}

		__forceinline operator T* ()
		{
			return decrypt();
		}

	private:
		mutable T _storage[_size]{};
		mutable bool _decrypted = false;
	};
}

#define skCrypt(str) []() { \
			using CharType = std::remove_const_t<std::remove_reference_t<decltype(str[0])>>; \
			static skc::skCrypter<sizeof(str) / sizeof(str[0]), __TIME__[4], __TIME__[7], CharType> crypted((CharType*)str); \
			return crypted.decrypt(); \
		}()
