#include "Decrypt.h"
#include <DMALibrary/Memory/Memory.h>
#include "common/Data.h"
#include "common/Offset.h"

DWORD Decrypt::CIndex(DWORD value)
{
    return ((((value ^ 0x7360F24) << 25) | (((value ^ 0x7360F24) >> 7) & 0x1FF0000)) ^ _rotr(value ^ 0x7360F24, 23) ^ 0xB621EC05);                
}

void Decrypt::DestroyXe()
{
    DecFunction = reinterpret_cast<uint64_t(*)(uint64_t key, uint64_t base)>(0);
}

uint64_t Decrypt::Xe(uint64_t addr)
{
    try
    {
        static uint64_t Ptr = 0;
        if (DecFunction == nullptr)
        {
            int64_t UWorld = 0x0;
            int64_t DecryptPtr = 0x0;

            DecryptPtr = mem.Read<uint64_t>((uint64_t)GameData.GameBase + Offset::XenuineDecrypt);
            UWorld = mem.Read<uint64_t>(GameData.GameBase + Offset::UWorld);

            if (!UWorld || !DecryptPtr)
            {
               return 0;
            }

            int32_t Tmp1Add = mem.Read<uint32_t>(DecryptPtr + 3);
            Ptr = Tmp1Add + DecryptPtr + 7;
            unsigned char ShellcodeBuff[1024] = { NULL };
            ShellcodeBuff[0] = 0x90;
            ShellcodeBuff[1] = 0x90;
            mem.Read(DecryptPtr, &ShellcodeBuff[2], sizeof(ShellcodeBuff) - 2);
            ShellcodeBuff[2] = 0x48;
            ShellcodeBuff[3] = 0x8B;
            ShellcodeBuff[4] = 0xC1;
            ShellcodeBuff[5] = 0x90;
            ShellcodeBuff[6] = 0x90;
            ShellcodeBuff[7] = 0x90;
            ShellcodeBuff[8] = 0x90;
            DecFunction = reinterpret_cast<decltype(DecFunction)> (VirtualAlloc(nullptr, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
            RtlCopyMemory((LPVOID)DecFunction, (LPVOID)ShellcodeBuff, sizeof(ShellcodeBuff));
            DWORD _oldP; VirtualProtect((LPVOID)DecFunction, 4096, PAGE_EXECUTE_READ, &_oldP);
        }

        return  DecFunction(Ptr, addr);
    }
    catch (const std::exception&)
    {
        return 0;
    }
}