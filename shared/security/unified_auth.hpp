#pragma once
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <intrin.h>
#include <ntddstor.h>
#include <winioctl.h>

#include "../nlohmann/json.hpp"
#include "skCrypt.h"
#include "WinSha256.h"
#include "WinCrypto.h"

#pragma comment(lib, "winhttp.lib")

namespace UnifiedAuth {

// RSA Public Key (RSASSA-PKCS1-v1_5)
const std::vector<BYTE> RSA_PUBLIC_KEY = {
    0x52, 0x53, 0x41, 0x31, 0x00, 0x04, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0xbf, 0x40, 0xea, 0xde, 0xa9, 
    0xf4, 0x7d, 0xfa, 0x39, 0x68, 0xee, 0x48, 0x34, 0x19, 0x34, 0x9b, 0x3e, 0xf0, 0xea, 0xcf, 0xf9, 
    0x55, 0x67, 0x1b, 0xf2, 0xb7, 0x8a, 0x81, 0xfc, 0x0b, 0xf3, 0x7c, 0x5b, 0xa6, 0x5a, 0xdf, 0xeb, 
    0x2a, 0x1d, 0x62, 0x66, 0x4b, 0x23, 0xaf, 0x92, 0x88, 0xb6, 0x15, 0xe3, 0xd6, 0xb0, 0x5f, 0x18, 
    0xad, 0xc5, 0x0b, 0x42, 0x15, 0xbf, 0x1c, 0x8b, 0xa9, 0xf0, 0x24, 0xd2, 0xed, 0x0b, 0x3d, 0xaf, 
    0x29, 0xb4, 0xca, 0x45, 0x8c, 0x95, 0x22, 0x4d, 0x5e, 0xc4, 0x9b, 0x0b, 0xd7, 0x88, 0x0f, 0x8a, 
    0xa8, 0x41, 0x9e, 0x11, 0x8b, 0x52, 0x78, 0x15, 0x33, 0x4e, 0xe7, 0x1b, 0x1a, 0x51, 0x9e, 0xbd, 
    0x49, 0x5b, 0xa1, 0xf0, 0x72, 0xab, 0x21, 0xe5, 0x39, 0x10, 0x20, 0x83, 0x98, 0x3a, 0x0a, 0x26, 
    0xe5, 0x85, 0x0d, 0x79, 0x1c, 0x9f, 0x24, 0xb5, 0x6c, 0x59, 0x69, 0x00, 0x00, 0x00, 0x00, 
};

class AuthState {
public:
    std::string active_key;
    std::string license_error;
    std::string expiry_str = "N/A";
    bool is_vietnamese = true;
    bool entitlement_active = false;
    time_t expiry_time = 0;
    uint64_t remaining_seconds = 0;
    uint64_t last_tick_count = 0;

    std::string account_token;
    std::string account_username;
    std::string account_role;
    std::string config_code;

    const wchar_t* api_host = skCrypt(L"licensing-backend.donghiem114.workers.dev");
    const wchar_t* path_activate = skCrypt(L"/loader/keys/activate");
    const wchar_t* path_heartbeat = skCrypt(L"/loader/heartbeat");
    const wchar_t* path_login = skCrypt(L"/loader/login");
    const wchar_t* path_launch_login = skCrypt(L"/loader/launch-login");
    const wchar_t* user_agent = skCrypt(L"GZ-Account-Loader");
    
    int signature_version = 2;
    long long signature_max_age_seconds = 300;

    void Clear() {
        active_key.clear();
        entitlement_active = false;
        expiry_str = "N/A";
        expiry_time = 0;
        remaining_seconds = 0;
        last_tick_count = 0;
    }

    bool HasActiveEntitlement() {
        return !account_token.empty() && entitlement_active && remaining_seconds > 0;
    }
};

static AuthState g_Auth;

inline std::wstring Utf8ToWide(const std::string& value) {
    if (value.empty()) return L"";
    int required = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
    if (required <= 0) return L"";
    std::wstring out((size_t)required - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, out.data(), required);
    return out;
}

inline std::string WideToUtf8(const std::wstring& value) {
    if (value.empty()) return "";
    int required = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (required <= 0) return "";
    std::string out((size_t)required - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, out.data(), required, nullptr, nullptr);
    return out;
}

inline std::string CommandLineOption(const wchar_t* optionName) {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) return "";

    std::wstring option(optionName);
    std::wstring prefix = option + L"=";
    std::string value;

    for (int i = 1; i < argc; ++i) {
        std::wstring arg = argv[i] ? argv[i] : L"";
        if (arg == option && i + 1 < argc) {
            value = WideToUtf8(argv[i + 1] ? argv[i + 1] : L"");
            break;
        }
        if (arg.rfind(prefix, 0) == 0) {
            value = WideToUtf8(arg.substr(prefix.size()));
            break;
        }
    }

    LocalFree(argv);
    return value;
}

inline bool IsHexLaunchToken(const std::string& value) {
    if (value.size() != 64) return false;
    for (char ch : value) {
        const bool digit = ch >= '0' && ch <= '9';
        const bool lower = ch >= 'a' && ch <= 'f';
        const bool upper = ch >= 'A' && ch <= 'F';
        if (!digit && !lower && !upper) return false;
    }
    return true;
}

inline std::string JsonStringValue(const nlohmann::json& json, const char* field) {
    auto it = json.find(field);
    return it != json.end() && it->is_string() ? it->get<std::string>() : "";
}

inline long long JsonInt64Value(const nlohmann::json& json, const char* field) {
    auto it = json.find(field);
    if (it == json.end()) return 0;
    if (it->is_number_integer()) return it->get<long long>();
    if (it->is_number_unsigned()) return (long long)it->get<unsigned long long>();
    return 0;
}

inline time_t ParseISO8601(const std::string& timestamp) {
    if (timestamp.empty()) return 0;
    struct tm tm = {0};
    std::string base_time = timestamp.substr(0, 19);
    std::istringstream ss(base_time);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) return 0;
    return _mkgmtime(&tm);
}

inline std::string GenerateSecureNonce(size_t byteCount = 32) {
    std::vector<BYTE> bytes(byteCount);
    HCRYPTPROV hCryptProv = 0;
    bool ok = false;
    if (CryptAcquireContextA(&hCryptProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        ok = CryptGenRandom(hCryptProv, (DWORD)bytes.size(), bytes.data()) == TRUE;
        CryptReleaseContext(hCryptProv, 0);
    }
    if (!ok) {
        LARGE_INTEGER perf;
        QueryPerformanceCounter(&perf);
        uint64_t seed = (uint64_t)perf.QuadPart ^ GetTickCount64() ^ ((uint64_t)GetCurrentProcessId() << 32);
        for (size_t i = 0; i < bytes.size(); ++i) {
            seed ^= seed << 13; seed ^= seed >> 7; seed ^= seed << 17;
            bytes[i] = (BYTE)(seed & 0xFF);
        }
    }
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (BYTE b : bytes) oss << std::setw(2) << (int)b;
    return oss.str();
}

inline bool HttpJsonRequest(const wchar_t* method, const wchar_t* path, const std::string& body, const std::string& bearerToken, std::string& responseStr) {
    responseStr.clear();
    HINTERNET hSession = WinHttpOpen(g_Auth.user_agent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;
    HINTERNET hConnect = WinHttpConnect(hSession, g_Auth.api_host, INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return false; }
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, method, path, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return false; }
    std::wstring headers = L"Content-Type: application/json\r\n";
    if (!bearerToken.empty()) {
        headers += L"Authorization: Bearer ";
        headers += Utf8ToWide(bearerToken);
        headers += L"\r\n";
    }
    LPVOID requestBody = body.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID)body.c_str();
    DWORD requestBodyLength = body.empty() ? 0 : (DWORD)body.length();
    bool ok = WinHttpSendRequest(hRequest, headers.c_str(), -1, requestBody, requestBodyLength, requestBodyLength, 0) == TRUE;
    if (ok) {
        ok = WinHttpReceiveResponse(hRequest, NULL) == TRUE;
        if (ok) {
            DWORD dwSize = 0;
            DWORD dwDownloaded = 0;
            do {
                dwSize = 0;
                if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
                if (!dwSize) break;
                std::vector<char> readBuffer(dwSize + 1, 0);
                if (WinHttpReadData(hRequest, readBuffer.data(), dwSize, &dwDownloaded)) {
                    responseStr.append(readBuffer.data(), dwDownloaded);
                }
            } while (dwSize > 0);
        }
    }
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return ok && !responseStr.empty();
}

inline bool HttpJsonPost(const wchar_t* path, const nlohmann::json& requestJson, const std::string& bearerToken, std::string& responseStr) {
    return HttpJsonRequest(L"POST", path, requestJson.dump(), bearerToken, responseStr);
}

inline std::string GetHWID() {
    static std::string cached_hwid = "";
    if (!cached_hwid.empty()) return cached_hwid;
    std::string hwid_raw = "";
    auto TrimStr = [](std::string s) -> std::string {
        s.erase(0, s.find_first_not_of(" \t\n\r\0"));
        s.erase(s.find_last_not_of(" \t\n\r\0") + 1);
        return s;
    };
    DWORD vol_serial = 0;
    if (GetVolumeInformationA(skCrypt("C:\\"), NULL, 0, &vol_serial, NULL, NULL, NULL, 0)) {
        char vol_str[32]; sprintf_s(vol_str, "%08X", vol_serial);
        hwid_raw += vol_str;
    }
    HANDLE hDrive = CreateFileA(skCrypt("\\\\.\\PhysicalDrive0"), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive != INVALID_HANDLE_VALUE) {
        STORAGE_PROPERTY_QUERY query = { StorageDeviceProperty, PropertyStandardQuery };
        DWORD bytesReturned = 0; STORAGE_DESCRIPTOR_HEADER header = { 0 };
        if (DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &header, sizeof(header), &bytesReturned, NULL)) {
            std::vector<BYTE> buffer(header.Size);
            if (DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), buffer.data(), (DWORD)buffer.size(), &bytesReturned, NULL)) {
                PSTORAGE_DEVICE_DESCRIPTOR pDesc = (PSTORAGE_DEVICE_DESCRIPTOR)buffer.data();
                if (pDesc->SerialNumberOffset != 0) hwid_raw += TrimStr((char*)(buffer.data() + pDesc->SerialNumberOffset));
            }
        }
        CloseHandle(hDrive);
    }
    int cpuinfo[4]; __cpuid(cpuinfo, 0x80000002);
    char cpu_str[64]; sprintf_s(cpu_str, "%08X%08X%08X%08X", cpuinfo[0], cpuinfo[1], cpuinfo[2], cpuinfo[3]);
    hwid_raw += cpu_str;
    HKEY hKey; char szBiosSerial[256] = { 0 }; DWORD dwBufLen = 256;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, skCrypt("HARDWARE\\DESCRIPTION\\System\\BIOS"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, skCrypt("BaseBoardSerialNumber"), NULL, NULL, (LPBYTE)szBiosSerial, &dwBufLen) == ERROR_SUCCESS)
            hwid_raw += TrimStr(std::string(szBiosSerial));
        RegCloseKey(hKey);
    }
    std::string hashed_raw = Sha::hmac_sha256(skCrypt("GZ_HARD_HWID_FINAL_V2"), hwid_raw);
    cached_hwid = hashed_raw.substr(0, 16);
    return cached_hwid;
}

inline bool ApplyExpiryFromJson(const nlohmann::json& json, const char* expiryField = "expiry", const char* timestampField = "timestamp") {
    std::string expiry = JsonStringValue(json, expiryField);
    if (expiry.empty()) expiry = JsonStringValue(json, "expires_at");
    const long long remaining = JsonInt64Value(json, "remaining_seconds");
    if (remaining > 0) {
        g_Auth.remaining_seconds = (uint64_t)remaining;
        g_Auth.last_tick_count = GetTickCount64();
        const time_t now = time(nullptr);
        g_Auth.expiry_time = now > 0 ? now + (time_t)remaining : 0;
        g_Auth.expiry_str = expiry.empty() ? skCrypt("ACTIVE") : expiry;
        return true;
    }
    std::string server_time = JsonStringValue(json, timestampField);
    if (server_time.empty()) server_time = JsonStringValue(json, "server_time");
    if (expiry.empty()) return false;
    time_t exp_t = ParseISO8601(expiry);
    time_t srv_t = server_time.empty() ? time(nullptr) : ParseISO8601(server_time);
    if (exp_t > srv_t) {
        g_Auth.remaining_seconds = (uint64_t)(exp_t - srv_t);
        g_Auth.last_tick_count = GetTickCount64();
        g_Auth.expiry_time = exp_t;
        g_Auth.expiry_str = expiry;
        return true;
    }
    g_Auth.remaining_seconds = 0;
    return false;
}

inline bool ValidateSignedLicensePayload(
    const nlohmann::json& responseJson,
    const std::string& expectedAction,
    const std::string& expectedStatus,
    const std::string& expectedKey,
    const std::string& expectedHwid,
    const std::string& expectedNonce,
    std::string& error
) {
    std::string signedPayload = JsonStringValue(responseJson, "signed_payload");
    std::string serverSignature = JsonStringValue(responseJson, "signature");
    if (signedPayload.empty() || serverSignature.empty()) { error = "SIGNED_PAYLOAD_MISSING"; return false; }
    if (!Crypto::VerifyRSASignature(RSA_PUBLIC_KEY, signedPayload, serverSignature)) { error = "SIGNATURE_INVALID"; return false; }
    nlohmann::json payloadJson = nlohmann::json::parse(signedPayload, nullptr, false);
    if (payloadJson.is_discarded() || !payloadJson.is_object()) { error = "SIGNED_PAYLOAD_INVALID_JSON"; return false; }
    if (JsonInt64Value(payloadJson, "v") != g_Auth.signature_version) { error = "SIGNED_PAYLOAD_VERSION_MISMATCH"; return false; }
    
    std::string signedStatus = JsonStringValue(payloadJson, "status");
    std::string signedKey = JsonStringValue(payloadJson, "key");
    const bool requireKeyMatch = expectedAction == "activate" || !expectedKey.empty();
    if (JsonStringValue(payloadJson, "action") != expectedAction ||
        signedStatus != expectedStatus ||
        (requireKeyMatch && signedKey != expectedKey) ||
        JsonStringValue(payloadJson, "hwid") != expectedHwid ||
        JsonStringValue(payloadJson, "nonce") != expectedNonce) {
        error = "SIGNED_PAYLOAD_FIELD_MISMATCH"; return false;
    }
    if (JsonStringValue(responseJson, "status") != signedStatus) { error = "RESPONSE_STATUS_MISMATCH"; return false; }
    long long issuedAt = JsonInt64Value(payloadJson, "issued_at");
    long long now = (long long)time(nullptr);
    long long age = now > issuedAt ? now - issuedAt : issuedAt - now;
    if (issuedAt <= 0 || age > g_Auth.signature_max_age_seconds) { error = "SIGNED_PAYLOAD_EXPIRED"; return false; }
    
    // Apply expiry from signed payload
    ApplyExpiryFromJson(payloadJson, "expiry", "timestamp");
    if (!signedKey.empty()) g_Auth.active_key = signedKey;
    return true;
}

inline bool DoAPIRequest(const std::string& key, const std::string& hwid, bool silent) {
    g_Auth.license_error = "";
    const wchar_t* endpointPath = silent ? g_Auth.path_heartbeat : g_Auth.path_activate;
    if (g_Auth.account_token.empty()) {
        g_Auth.license_error = g_Auth.is_vietnamese ? "CHUA DANG NHAP TAI KHOAN." : "ACCOUNT IS NOT LOGGED IN.";
        return false;
    }
    std::string nonce = GenerateSecureNonce();
    nlohmann::json requestJson;
    if (!key.empty()) requestJson["key"] = key;
    requestJson["hwid"] = hwid;
    requestJson["nonce"] = nonce;
    requestJson["signature_version"] = g_Auth.signature_version;
    std::string responseStr = "";
    if (!HttpJsonPost(endpointPath, requestJson, g_Auth.account_token, responseStr)) {
        if (!silent) g_Auth.license_error = g_Auth.is_vietnamese ? "Loi ket noi may chu." : "Connection error.";
        return false;
    }
    nlohmann::json responseJson = nlohmann::json::parse(responseStr, nullptr, false);
    if (responseJson.is_discarded() || !responseJson.is_object()) {
        if (!silent) g_Auth.license_error = g_Auth.is_vietnamese ? "Phan hoi khong hop le." : "Invalid response.";
        return false;
    }
    std::string status = JsonStringValue(responseJson, "status");
    const std::string expectedAction = silent ? "heartbeat" : "activate";
    const std::string expectedStatus = silent ? "OK" : "ACTIVATED";
    if (status == expectedStatus) {
        std::string verifyError;
        if (!ValidateSignedLicensePayload(responseJson, expectedAction, expectedStatus, key, hwid, nonce, verifyError)) {
            if (!silent) g_Auth.license_error = (g_Auth.is_vietnamese ? "Xac thuc toan ven that bai! " : "Integrity check failed! ") + verifyError;
            return false;
        }
        std::string responseConfigCode = JsonStringValue(responseJson, "config_code");
        if (!responseConfigCode.empty()) g_Auth.config_code = responseConfigCode;
        g_Auth.entitlement_active = g_Auth.remaining_seconds > 0;
        return true;
    }
    g_Auth.license_error = status;
    return false;
}

inline bool ParseAuthSessionResponse(const std::string& responseStr, bool allowNoActiveKey) {
    nlohmann::json responseJson = nlohmann::json::parse(responseStr, nullptr, false);
    if (responseJson.is_discarded() || !responseJson.is_object()) return false;
    if (responseJson.contains("error")) { g_Auth.license_error = JsonStringValue(responseJson, "error"); return false; }
    std::string topStatus = JsonStringValue(responseJson, "status");
    if (topStatus == "SESSION_REPLACED" || topStatus == "SESSION_HWID_MISMATCH" || topStatus == "BANNED") {
        g_Auth.license_error = topStatus; g_Auth.account_token.clear(); g_Auth.Clear(); return false;
    }
    if (!topStatus.empty() && topStatus != "OK" && topStatus != "ACTIVATED") { g_Auth.license_error = topStatus; return false; }
    std::string token = JsonStringValue(responseJson, "token");
    if (!token.empty()) g_Auth.account_token = token;
    auto userIt = responseJson.find("user");
    if (userIt != responseJson.end() && userIt->is_object()) {
        g_Auth.account_username = JsonStringValue(*userIt, "username");
        g_Auth.account_role = JsonStringValue(*userIt, "role");
    }
    g_Auth.config_code = JsonStringValue(responseJson, "config_code");
    auto entitlementIt = responseJson.find("entitlement");
    if (entitlementIt != responseJson.end() && entitlementIt->is_object()) {
        std::string key = JsonStringValue(*entitlementIt, "key");
        bool active = (*entitlementIt).value("active", false);
        if (active && ApplyExpiryFromJson(*entitlementIt, "expires_at", "timestamp")) {
            if (!key.empty()) g_Auth.active_key = key;
            g_Auth.entitlement_active = true;
        } else g_Auth.Clear();
        return g_Auth.HasActiveEntitlement() || allowNoActiveKey;
    }
    g_Auth.Clear();
    return allowNoActiveKey && (!token.empty());
}

inline bool LoginAccount(const std::string& username, const std::string& password, const std::string& hwid) {
    g_Auth.license_error.clear();
    nlohmann::json requestJson;
    requestJson["username"] = username;
    requestJson["password"] = password;
    requestJson["hwid"] = hwid;
    std::string responseStr;
    if (!HttpJsonPost(g_Auth.path_login, requestJson, "", responseStr)) {
        g_Auth.license_error = g_Auth.is_vietnamese ? "Khong ket noi duoc may chu tai khoan." : "Cannot connect to account server.";
        return false;
    }
    if (!ParseAuthSessionResponse(responseStr, true) || g_Auth.account_token.empty()) {
        if (g_Auth.license_error.empty()) g_Auth.license_error = responseStr;
        return false;
    }
    return true;
}

inline bool TryLaunchTokenLogin(const std::string& hwid) {
    g_Auth.license_error.clear();
    const std::string launchToken = CommandLineOption(L"--launch-token");
    if (!IsHexLaunchToken(launchToken)) return false;

    nlohmann::json requestJson;
    requestJson["launch_token"] = launchToken;
    requestJson["hwid"] = hwid;

    std::string responseStr;
    if (!HttpJsonPost(g_Auth.path_launch_login, requestJson, "", responseStr)) {
        g_Auth.license_error = g_Auth.is_vietnamese ? "Khong ket noi duoc may chu launch token." : "Cannot connect to launch token server.";
        return false;
    }

    if (!ParseAuthSessionResponse(responseStr, true) || g_Auth.account_token.empty()) {
        if (g_Auth.license_error.empty()) g_Auth.license_error = responseStr;
        return false;
    }
    return true;
}

inline bool PromptAccountLogin() {
    std::string username;
    std::string password;
    std::cout << skCrypt("Tai khoan: ");
    std::getline(std::cin, username);
    std::cout << skCrypt("Mat khau: ");
    std::getline(std::cin, password);
    return LoginAccount(username, password, GetHWID());
}

inline bool AuthenticateWithKey(const std::string& key) {
    return DoAPIRequest(key, GetHWID(), false);
}

inline bool EnsureAccountSession() {
    const std::string hwid = GetHWID();
    if (TryLaunchTokenLogin(hwid)) return true;
    if (!g_Auth.account_token.empty()) return true;
    return PromptAccountLogin();
}

inline bool EnsureActiveLicenseForConsole(const char* keyFilePath = "key.txt") {
    if (!EnsureAccountSession()) return false;
    if (g_Auth.HasActiveEntitlement()) return true;

    std::string key;
    std::ifstream keyFile(keyFilePath);
    if (keyFile.is_open()) {
        std::getline(keyFile, key);
        keyFile.close();
    }

    if (!key.empty()) {
        if (AuthenticateWithKey(key)) return true;
        key.clear();
    }

    std::cout << skCrypt("License Key: ");
    std::getline(std::cin, key);
    if (!AuthenticateWithKey(key)) return false;

    std::ofstream saveFile(keyFilePath);
    if (saveFile.is_open()) saveFile << key;
    return true;
}

inline void LicenseHeartbeatLoop() {
    std::string hwid = GetHWID();
    uint64_t last_net_check_tick = GetTickCount64();
    uint64_t next_check_ms = (180 + (rand() % 121)) * 1000;

    while (true) {
        Sleep(5000);
        uint64_t current_tick = GetTickCount64();
        uint64_t elapsed_seconds = (current_tick - g_Auth.last_tick_count) / 1000;

        if (g_Auth.remaining_seconds > 0 && elapsed_seconds >= g_Auth.remaining_seconds) {
            std::cout << (g_Auth.is_vietnamese ? "\n[!!!] HET HAN SU DUNG.\n" : "\n[!!!] SUBSCRIPTION EXPIRED.\n");
            Sleep(3000);
            exit(1);
        }

        if ((current_tick - last_net_check_tick) >= next_check_ms) {
            if (!g_Auth.account_token.empty()) {
                if (!DoAPIRequest(g_Auth.active_key, hwid, true)) {
                    std::cout << (g_Auth.is_vietnamese ? "\n[!!!] PHIEN DANG NHAP LOI.\n" : "\n[!!!] SESSION ERROR.\n");
                    Sleep(3000);
                    exit(1);
                }
            }
            last_net_check_tick = current_tick;
            next_check_ms = (180 + (rand() % 121)) * 1000;
        }
    }
}

} // namespace UnifiedAuth
