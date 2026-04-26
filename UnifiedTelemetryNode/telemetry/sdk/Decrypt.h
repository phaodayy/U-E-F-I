#pragma once

#include <Windows.h>
#include <cstdint>

static auto DecFunction = reinterpret_cast<uint64_t(*)(uint64_t key, uint64_t base)>(0);

class Decrypt
{
public:
	static DWORD CIndex(DWORD value);
	static uint64_t Xe(uint64_t addr);
	static void DestroyXe();
};
