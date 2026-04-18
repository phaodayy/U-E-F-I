#pragma once
#include <string>
#include <Windows.h>
#include <wincrypt.h>

namespace mapper::secret {
    // Derive BUILD_KEY at runtime from HWID instead of hardcoding
    inline std::string derive_build_key() {
        DWORD vol_serial = 0;
        GetVolumeInformationA("C:\\", NULL, 0, &vol_serial, NULL, NULL, NULL, 0);

        int cpuinfo[4];
        __cpuid(cpuinfo, 1);

        // Mix hardware values into a deterministic key
        uint8_t seed[16];
        memcpy(seed, &vol_serial, 4);
        memcpy(seed + 4, &cpuinfo[0], 4);
        memcpy(seed + 8, &cpuinfo[3], 4);
        uint32_t mix = vol_serial ^ cpuinfo[0] ^ cpuinfo[3];
        memcpy(seed + 12, &mix, 4);

        // SHA-256 hash the seed to produce a stable 32-char key
        HCRYPTPROV hProv = 0; HCRYPTHASH hHash = 0;
        std::string key(32, '\0');
        if (CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
            if (CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
                CryptHashData(hHash, seed, sizeof(seed), 0);
                BYTE hash[32]; DWORD hashLen = 32;
                CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0);
                const char hex[] = "0123456789abcdef";
                for (int i = 0; i < 16; i++) {
                    key[i * 2]     = hex[hash[i] >> 4];
                    key[i * 2 + 1] = hex[hash[i] & 0xF];
                }
                CryptDestroyHash(hHash);
            }
            CryptReleaseContext(hProv, 0);
        }
        return key;
    }

    inline std::string BUILD_KEY = derive_build_key();
    inline uint32_t SESSION_TOKEN = 0;
}
