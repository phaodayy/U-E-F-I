#pragma once
#include <Windows.h>
#include <fltUser.h>
#include <iostream>
#include <vector>
#include <string>
#include <intrin.h>

#pragma comment(lib, "FltLib.lib")

#define OpCode_ReadVM 14
#define OpCode_WriteVM 70

namespace NetEaseMemory {
    inline HANDLE g_PortHandle = INVALID_HANDLE_VALUE;
    inline BYTE g_Key[33] = "FuckKeenFuckKeenFuckKeenFuckKeen";
    inline unsigned char g_NetEaseSafe_EncImm[] =
    {
        0x7A, 0x54, 0xE5, 0x41, 0x8B, 0xDB, 0xB0, 0x55, 0x7A, 0xBD,
        0x01, 0xBD, 0x1A, 0x7F, 0x9E, 0x17
    };

    typedef struct _NEAC_FILTER_CONNECT {
        DWORD Magic;
        DWORD Version;
        BYTE EncKey[32];
    } NEAC_FILTER_CONNECT, * PNEAC_FILTER_CONNECT;

    typedef struct _NEAC_READ_PACKET {
        BYTE Opcode;
        PVOID Src;
        DWORD Size;
    } NEAC_READ_PACKET, * PNEAC_READ_PACKET;

    typedef struct _NEAC_WRITE_PACKET {
        BYTE Opcode;
        PVOID Dst;
        PVOID Src;
        DWORD Size;
    } NEAC_WRITE_PACKET, * PNEAC_WRITE_PACKET;

    inline void NetEaseEncyptBuffer(unsigned int* buffer, unsigned int idx)
    {
        __m128i v2;
        unsigned int* result;
        int v4;
        __m128i v5;
        __m128i v8;

        __m128i imm = _mm_load_si128((__m128i*)g_NetEaseSafe_EncImm);
        __m128i zero;
        memset(&zero, 0, sizeof(__m128i));
        v2 = _mm_cvtsi32_si128(idx);
        
        // Simulating the result pointer logic from the disassembly
        alignas(16) unsigned int v8_buf[4];
        v8 = _mm_xor_si128(
            _mm_shuffle_epi32(_mm_shufflelo_epi16(_mm_unpacklo_epi8(v2, v2), 0), 0),
            imm);
        _mm_store_si128((__m128i*)v8_buf, v8);

        result = &v8_buf[3];
        v4 = 0;
        v5 = _mm_cvtsi32_si128(0x4070E1Fu);
        do
        {
            __m128i v6 = _mm_shufflelo_epi16(_mm_unpacklo_epi8(_mm_or_si128(_mm_cvtsi32_si128(*result), v5), zero), 27);
            v6 = _mm_packus_epi16(v6, v6);
            
            alignas(16) unsigned int v6_buf[4];
            _mm_store_si128((__m128i*)v6_buf, v6);

            *buffer = (*buffer ^ ~idx) ^ v6_buf[0] ^ idx;
            ++buffer;
            result = (unsigned int*)((char*)result - 1);
            v4++;
        } while (v4 < 4);
    }

    inline void NetEaseSafeEncodePayload(PBYTE key, PBYTE buffer, SIZE_T size)
    {
        for (size_t i = 0; i < size; i++) {
            buffer[i] ^= key[i & 31];
        }
        unsigned int* ptr = (unsigned int*)buffer;
        unsigned int v12 = 0;
        do
        {
            NetEaseEncyptBuffer(ptr, v12++);
            ptr += 4;
        } while (v12 < size >> 4);
    }

    inline bool Initialize() {
        if (g_PortHandle != INVALID_HANDLE_VALUE) return true;

        const wchar_t* portNames[] = { L"\\NeacClient", L"\\NEAntiCheat", L"\\PYSafe", L"\\NeacSafePort", L"\\NeacSafe64", L"\\NeacSafe64Port" };
        HRESULT hResult = S_OK;

        for (const auto& name : portNames) {
            NEAC_FILTER_CONNECT lpContext = { 0 };
            lpContext.Magic = 0x4655434B; // "FUCK"
            lpContext.Version = 8;
            memcpy(lpContext.EncKey, g_Key, 32);

            hResult = FilterConnectCommunicationPort(name, 0, &lpContext, sizeof(lpContext), NULL, &g_PortHandle);
            if (hResult == S_OK) {
                std::wcout << L"\n[+] Connected to port: " << name << std::endl;
                return true;
            }

            // Try with Magic "TUCK" and alternative version
            lpContext.Magic = 0x5455434B; // "TUCK"
            lpContext.Version = 10;
            hResult = FilterConnectCommunicationPort(name, 0, &lpContext, sizeof(lpContext), NULL, &g_PortHandle);
            if (hResult == S_OK) {
                std::wcout << L"\n[+] Connected to port (TUCK v10): " << name << std::endl;
                return true;
            }
        }

        std::cout << "\n[-] FilterConnectCommunicationPort failed! HRESULT: 0x" << std::hex << hResult << std::dec << std::endl;
        g_PortHandle = INVALID_HANDLE_VALUE;
        return false;
    }

    inline bool ReadMemory(uint64_t address, void* buffer, uint32_t size) {
        if (g_PortHandle == INVALID_HANDLE_VALUE) return false;

        DWORD bytesReturned;
        alignas(16) BYTE packetBuffer[16];
        NEAC_READ_PACKET* ptr = (NEAC_READ_PACKET*)packetBuffer;

        ptr->Opcode = OpCode_ReadVM;
        ptr->Src = (PVOID)address;
        ptr->Size = size;

        NetEaseSafeEncodePayload(g_Key, packetBuffer, sizeof(packetBuffer));
        HRESULT hr = FilterSendMessage(g_PortHandle, packetBuffer, sizeof(packetBuffer), buffer, size, &bytesReturned);
        return SUCCEEDED(hr);
    }

    inline bool WriteMemory(uint64_t address, void* buffer, uint32_t size) {
        if (g_PortHandle == INVALID_HANDLE_VALUE) return false;

        DWORD bytesReturned;
        alignas(16) BYTE packetBuffer[32];
        NEAC_WRITE_PACKET* ptr = (NEAC_WRITE_PACKET*)packetBuffer;

        ptr->Opcode = OpCode_WriteVM;
        ptr->Dst = (PVOID)address;
        ptr->Src = buffer;
        ptr->Size = size;

        NetEaseSafeEncodePayload(g_Key, packetBuffer, sizeof(packetBuffer));
        HRESULT hr = FilterSendMessage(g_PortHandle, packetBuffer, sizeof(packetBuffer), NULL, 0, &bytesReturned);
        return SUCCEEDED(hr);
    }

    template <typename T>
    inline T Read(uint64_t address) {
        T buffer = {};
        ReadMemory(address, &buffer, sizeof(T));
        return buffer;
    }
}
