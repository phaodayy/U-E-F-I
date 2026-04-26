#pragma once
#include <windows.h>
#include <bcrypt.h>
#include <vector>
#include <string>
#include <wincrypt.h>

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "crypt32.lib")

namespace Crypto {
    // Helper to decode Base64 string to bytes
    inline std::vector<BYTE> Base64Decode(const std::string& input) {
        const std::string b64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::vector<BYTE> out;
        std::vector<int> T(256, -1);
        for (int i = 0; i < 64; i++) T[b64chars[i]] = i;
        int val = 0, valb = -8;
        for (unsigned char c : input) {
            if (T[c] == -1) continue;
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0) {
                out.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return out;
    }

    // Verify RSA Signature (RSASSA-PKCS1-v1_5 with SHA256)
    inline bool VerifyRSASignature(const std::vector<BYTE>& publicKeyBlob, const std::string& data, const std::string& signatureBase64) {
        BCRYPT_ALG_HANDLE hAlg = NULL;
        BCRYPT_KEY_HANDLE hKey = NULL;
        BCRYPT_ALG_HANDLE hHashAlg = NULL;
        BCRYPT_HASH_HANDLE hHash = NULL;

        std::vector<BYTE> signature = Base64Decode(signatureBase64);
        if (signature.empty()) return false;

        std::vector<BYTE> safeBlob = publicKeyBlob;
        if (safeBlob.size() > 155) safeBlob.resize(155);

        bool verified = false;

        if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RSA_ALGORITHM, NULL, 0) == 0) {
            if (BCryptImportKeyPair(hAlg, NULL, BCRYPT_RSAPUBLIC_BLOB, &hKey, safeBlob.data(), (ULONG)safeBlob.size(), 0) == 0) {
                if (BCryptOpenAlgorithmProvider(&hHashAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0) == 0) {
                    if (BCryptCreateHash(hHashAlg, &hHash, NULL, 0, NULL, 0, 0) == 0) {
                        if (BCryptHashData(hHash, (PUCHAR)data.c_str(), (ULONG)data.length(), 0) == 0) {
                            BYTE hash[32];
                            if (BCryptFinishHash(hHash, hash, 32, 0) == 0) {
                                BCRYPT_PKCS1_PADDING_INFO padInfo = {};
                                padInfo.pszAlgId = BCRYPT_SHA256_ALGORITHM;

                                NTSTATUS status = BCryptVerifySignature(hKey, &padInfo, hash, 32, signature.data(), (ULONG)signature.size(), BCRYPT_PAD_PKCS1);
                                if (status == 0) {
                                    verified = true;
                                }
                            }
                        }
                        BCryptDestroyHash(hHash);
                    }
                    BCryptCloseAlgorithmProvider(hHashAlg, 0);
                }
                BCryptDestroyKey(hKey);
            }
            BCryptCloseAlgorithmProvider(hAlg, 0);
        }

        return verified;
    }
}
