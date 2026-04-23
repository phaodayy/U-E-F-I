#pragma once
#include <windows.h>
#include <bcrypt.h>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>

#pragma comment(lib, "bcrypt.lib")

namespace Sha {
    inline std::string hmac_sha256(const std::string& key, const std::string& input) {
        BCRYPT_ALG_HANDLE hAlg = NULL;
        BCRYPT_HASH_HANDLE hHash = NULL;
        NTSTATUS status = 0;
        DWORD cbData = 0, cbHash = 0, cbHashObject = 0;
        PBYTE pbHashObject = NULL;
        PBYTE pbHash = NULL;

        status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, BCRYPT_ALG_HANDLE_HMAC_FLAG);
        if (status < 0) return "";

        status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0);
        if (status < 0) { BCryptCloseAlgorithmProvider(hAlg, 0); return ""; }

        pbHashObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHashObject);
        if (NULL == pbHashObject) { BCryptCloseAlgorithmProvider(hAlg, 0); return ""; }

        status = BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PBYTE)&cbHash, sizeof(DWORD), &cbData, 0);
        if (status < 0) { HeapFree(GetProcessHeap(), 0, pbHashObject); BCryptCloseAlgorithmProvider(hAlg, 0); return ""; }

        pbHash = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHash);
        if (NULL == pbHash) { HeapFree(GetProcessHeap(), 0, pbHashObject); BCryptCloseAlgorithmProvider(hAlg, 0); return ""; }

        status = BCryptCreateHash(hAlg, &hHash, pbHashObject, cbHashObject, (PBYTE)key.c_str(), (ULONG)key.length(), 0);
        if (status < 0) { HeapFree(GetProcessHeap(), 0, pbHash); HeapFree(GetProcessHeap(), 0, pbHashObject); BCryptCloseAlgorithmProvider(hAlg, 0); return ""; }

        status = BCryptHashData(hHash, (PBYTE)input.c_str(), (ULONG)input.length(), 0);
        if (status < 0) { BCryptDestroyHash(hHash); HeapFree(GetProcessHeap(), 0, pbHash); HeapFree(GetProcessHeap(), 0, pbHashObject); BCryptCloseAlgorithmProvider(hAlg, 0); return ""; }

        status = BCryptFinishHash(hHash, pbHash, cbHash, 0);
        if (status < 0) { BCryptDestroyHash(hHash); HeapFree(GetProcessHeap(), 0, pbHash); HeapFree(GetProcessHeap(), 0, pbHashObject); BCryptCloseAlgorithmProvider(hAlg, 0); return ""; }

        std::stringstream ss;
        for (DWORD i = 0; i < cbHash; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)pbHash[i];
        }

        BCryptDestroyHash(hHash);
        HeapFree(GetProcessHeap(), 0, pbHash);
        HeapFree(GetProcessHeap(), 0, pbHashObject);
        BCryptCloseAlgorithmProvider(hAlg, 0);

        return ss.str();
    }
}
