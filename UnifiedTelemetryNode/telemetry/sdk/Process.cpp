#include "Process.h"
#include <Common/Data.h>
#include <TlHelp32.h>
#include <Psapi.h>

HANDLE Process::Handle = INVALID_HANDLE_VALUE;
uintptr_t Process::ImageBase = 0;
uint32_t Process::ImageSize = 0;
uint32_t Process::ID = 0;
uint8_t* Process::Dump = nullptr;

bool Process::Init(const std::wstring Process_name)
{
    // [SECURITY] This class legacy code used OpenProcess(PROCESS_ALL_ACCESS).
    // It has been disabled to prevent BattlEye detection.
    // Use telemetryMemory and Hypervisor context instead.
    
    /*
	if (!Utils::ValidPtr(GameData.GameBase) || !Utils::ValidPtr(GameData.UWorld) || !Utils::ValidPtr(GameData.ActorArray)) return false;

	IMAGE_DOS_HEADER DosHeader = mem.Read<IMAGE_DOS_HEADER>(GameData.GameBase);
	IMAGE_NT_HEADERS NtHeader = mem.Read<IMAGE_NT_HEADERS>((GameData.GameBase + DosHeader.e_lfanew));

	ImageSize = NtHeader.OptionalHeader.SizeOfImage;
	Dump = new uint8_t[Process::ImageSize];

	mem.Read(GameData.GameBase, (void*)(Dump), ImageSize);
    */
	return true; 
}