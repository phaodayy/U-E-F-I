#ifndef _AMD64_
#define _AMD64_
#endif
#define _CRT_USE_WINAPI_FAMILY_DESKTOP_APP
#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <winioctl.h>
#include <ntddstor.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <cstdint>
#include <iostream>
#include <winternl.h>
#include <vector>
#include <thread>
#include <string>
#include <fstream>
#include <winhttp.h>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <cstdio>
#include "../nlohmann/json.hpp"

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "psapi.lib")

#include ".shared/shared.hpp"
#include "sdk/memory/memory.hpp"

typedef struct _telemetry_SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    BYTE Reserved1[48];
    UNICODE_STRING ImageName;
    KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    PVOID Reserved2;
    ULONG HandleCount;
    ULONG SessionId;
    PVOID Reserved3;
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG Reserved4;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    PVOID Reserved5;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER Reserved6[6];
} telemetry_SYSTEM_PROCESS_INFORMATION, *Ptelemetry_SYSTEM_PROCESS_INFORMATION;

#include <protec/skCrypt.h>
#include "sdk/core/app_shutdown.hpp"
#include "sdk/core/app_paths.hpp"
#include "sdk/memory/hyper_process.hpp"
#include "sdk/core/process_single_instance.hpp"
#include "sdk/core/offsets.hpp"
#include "sdk/core/context.hpp"
#include "sdk/core/telemetry_decrypt.hpp"
#include "sdk/utils/MacroEngine.h"
#include "overlay/core/overlay_menu.hpp"
#include "overlay/core/discord_overlay.h"
#include "sdk/memory/netease_comm.hpp"
#include "sdk/memory/EptMouseSetup.hpp"

#include "sdk/Utils/WinSha256.h"
#include "sdk/Utils/WinCrypto.h"
#include "sdk/Utils/ADVobfuscator.h"
#include <protec/protector.h>

// [GZ-DEBUG] Force OBF_STR evasion_calibration after all includes to prevent template errors in Debug mode
#ifdef _DEBUG
#undef OBF_STR
#define OBF_STR(x) x
#endif

// SECURITY V3: RSA Public Key (RSASSA-PKCS1-v1_5)
// This key replaces the vulnerable HMAC salt.
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

#include <intrin.h>

// Session token (replaces mapper::secret::SESSION_TOKEN)
static uint32_t g_session_token = 0;

std::string GetHWID() {
    static std::string cached_hwid = "";
    if (!cached_hwid.empty()) return cached_hwid;

    std::string hwid_raw = "";

    // [STEALTH V5] Retrieve stable hardware seed from Ring -1 (Hypervisor)
    // This provides an immutable identifier that usermode/kernel-mode spoofers cannot reach.
    uint64_t hv_seed = telemetryHyperCall::GetHardwareFingerprint();
    if (hv_seed != 0) {
        char hv_str[32];
        sprintf_s(hv_str, skCrypt("%016llX"), hv_seed);
        hwid_raw += hv_str;
        // If we have a solid HV seed, we can trust it more.
    }

    // Helper: Trim whitespace from serial strings (fixes inconsistent reads)
    auto TrimStr = [](std::string s) -> std::string {
        s.erase(0, s.find_first_not_of(" \t\n\r\0"));
        s.erase(s.find_last_not_of(" \t\n\r\0") + 1);
        return s;
    };

    // 1. Volume Serial Number (Ổ đĩa C) - Ổn định trừ khi format lại
    DWORD vol_serial = 0;
    if (GetVolumeInformationA(skCrypt("C:\\"), NULL, 0, &vol_serial, NULL, NULL, NULL, 0)) {
        char vol_str[32];
        sprintf_s(vol_str, skCrypt("%08X"), vol_serial);
        hwid_raw += vol_str;
    } else {
        hwid_raw += skCrypt("NOVOL");
    }

    // 2. Disk Physical Serial Number (SSD/HDD - Khó spoof)
    HANDLE hDrive = CreateFileA(skCrypt("\\\\.\\PhysicalDrive0"), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive != INVALID_HANDLE_VALUE) {
        STORAGE_PROPERTY_QUERY query = { StorageDeviceProperty, PropertyStandardQuery };
        DWORD bytesReturned = 0;
        STORAGE_DESCRIPTOR_HEADER header = { 0 };
        if (DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &header, sizeof(header), &bytesReturned, NULL)) {
            std::vector<BYTE> buffer(header.Size);
            if (DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), buffer.data(), (DWORD)buffer.size(), &bytesReturned, NULL)) {
                PSTORAGE_DEVICE_DESCRIPTOR pDesc = (PSTORAGE_DEVICE_DESCRIPTOR)buffer.data();
                if (pDesc->SerialNumberOffset != 0) {
                    std::string diskSerial = TrimStr((char*)(buffer.data() + pDesc->SerialNumberOffset));
                    hwid_raw += diskSerial;
                }
            }
        }
        CloseHandle(hDrive);
    } else {
        hwid_raw += skCrypt("NODISK");
    }

    // 3. CPU ID (Leaf 0x80000002..4 for Brand String - Stable)
    int cpuinfo[4];
    __cpuid(cpuinfo, 0x80000002);
    char cpu_str[64];
    sprintf_s(cpu_str, skCrypt("%08X%08X%08X%08X"), cpuinfo[0], cpuinfo[1], cpuinfo[2], cpuinfo[3]);
    hwid_raw += cpu_str;

    // 4. BIOS/Mainboard Serial (Registry)
    HKEY hKey;
    char szBiosSerial[256] = { 0 };
    DWORD dwBufLen = 256;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, skCrypt("HARDWARE\\DESCRIPTION\\System\\BIOS"), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, skCrypt("BaseBoardSerialNumber"), NULL, NULL, (LPBYTE)szBiosSerial, &dwBufLen) == ERROR_SUCCESS) {
            hwid_raw += TrimStr(std::string(szBiosSerial));
        } else {
            hwid_raw += skCrypt("NOBIOS");
        }
        RegCloseKey(hKey);
    } else {
        hwid_raw += skCrypt("NOREG");
    }

#ifdef _DEBUG
    // std::cout << "[DEBUG] HWID RAW: " << hwid_raw << std::endl;
#endif

    // Hash - 16 ký tự để giảm collision
    std::string hashed_raw = Sha::hmac_sha256(skCrypt("GZ_HARD_HWID_FINAL_V2"), hwid_raw);
    cached_hwid = hashed_raw.substr(0, 16);
    return cached_hwid;
}

std::string GetCurrentBinaryHash() {
    char szPath[MAX_PATH];
    GetModuleFileNameA(NULL, szPath, MAX_PATH);
    std::ifstream file(szPath, std::ios::binary);
    if (!file.is_open()) return skCrypt("");

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Use null salt for plain SHA256
    return Sha::hmac_sha256(skCrypt("GZ_SHA256_SALT_V4"), std::string(buffer.begin(), buffer.end()));
}

std::string global_active_key = skCrypt("");
std::string global_license_error = skCrypt("");
std::string g_expiry_str = skCrypt("N/A");
bool g_is_vietnamese = false;
time_t g_expiry_time = 0;
uint64_t g_remaining_seconds = 0;
uint64_t g_last_tick_count = 0;

extern const wchar_t* LICENSE_API_HOST = L"licensing-backend.donghiem114.workers.dev";
extern const wchar_t* LICENSE_ACTIVATE_PATH = L"/public/activate";
extern const wchar_t* LICENSE_HEARTBEAT_PATH = L"/public/heartbeat";
extern const wchar_t* LOADER_REGISTER_PATH = L"/loader/register";
extern const wchar_t* LOADER_LOGIN_PATH = L"/loader/login";
extern const wchar_t* LOADER_ME_PATH = L"/loader/me";
extern const wchar_t* LOADER_KEY_ACTIVATE_PATH = L"/loader/keys/activate";
extern const wchar_t* LOADER_HEARTBEAT_PATH = L"/loader/heartbeat";
extern const wchar_t* LOADER_CONFIG_PATH = L"/loader/config";
extern const wchar_t* LOADER_CONFIG_IMPORT_PATH = L"/loader/config/import";
constexpr const wchar_t* LICENSE_USER_AGENT = L"GZ-Account-Loader";
constexpr int LICENSE_SIGNATURE_VERSION = 2;
constexpr long long LICENSE_SIGNATURE_MAX_AGE_SECONDS = 300;
std::string global_account_token = skCrypt("");
std::string global_account_username = skCrypt("");
std::string global_account_role = skCrypt("");
std::string global_config_code = skCrypt("");

void ClearActiveEntitlementState() {
    global_active_key.clear();
    g_expiry_str = skCrypt("N/A");
    g_expiry_time = 0;
    g_remaining_seconds = 0;
    g_last_tick_count = 0;
}

bool HasActiveLoaderEntitlement() {
    return !global_account_token.empty() && !global_active_key.empty();
}

// Chuyển "2026-04-02T16:17:30" => time_t (UTC)
time_t ParseISO8601(const std::string& timestamp) {
    if (timestamp.empty()) return 0;
    struct tm tm = {0};
    // Lấy chuỗi YYYY-MM-DDTHH:MM:SS
    std::string base_time = timestamp.substr(0, 19);
    std::istringstream ss(base_time);
    ss >> std::get_time(&tm, skCrypt("%Y-%m-%dT%H:%M:%S"));
    if (ss.fail()) return 0;
    return _mkgmtime(&tm);
}
void SetConsoleColor(int color) {
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void TypewriterPrint(const std::string& text, int delay_ms = 20, int color = 7) {
  SetConsoleColor(color);
  for (char c : text) {
    std::cout << c << std::flush;
    Sleep(delay_ms);
  }
  SetConsoleColor(7);
}

void EnsureLoaderConsole() {
#ifdef _DEBUG
    if (GetConsoleWindow()) return;
    if (!AllocConsole()) return;

    FILE* stream = nullptr;
    freopen_s(&stream, skCrypt("CONOUT$"), skCrypt("w"), stdout);
    freopen_s(&stream, skCrypt("CONOUT$"), skCrypt("w"), stderr);
    freopen_s(&stream, skCrypt("CONIN$"), skCrypt("r"), stdin);
    SetConsoleTitleA(skCrypt("GZ Loader"));
#endif
}

void DebugPause() {
#ifdef _DEBUG
    system(skCrypt("pause"));
#endif
}

// --- LINKER CONFIG: WINDOWS FOR RELEASE, CONSOLE FOR DEBUG ---
#ifdef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#else
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

void SelectLanguage() {
    g_is_vietnamese = true;
}

std::string GenerateSecureNonce(size_t byteCount = 32) {
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
            seed ^= seed << 13;
            seed ^= seed >> 7;
            seed ^= seed << 17;
            bytes[i] = (BYTE)(seed & 0xFF);
        }
    }

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (BYTE b : bytes) {
        oss << std::setw(2) << (int)b;
    }
    return oss.str();
}

std::string JsonStringValue(const nlohmann::json& json, const char* field) {
    auto it = json.find(field);
    return it != json.end() && it->is_string() ? it->get<std::string>() : skCrypt("");
}

long long JsonInt64Value(const nlohmann::json& json, const char* field) {
    auto it = json.find(field);
    if (it == json.end()) return 0;
    if (it->is_number_integer()) return it->get<long long>();
    if (it->is_number_unsigned()) return (long long)it->get<unsigned long long>();
    return 0;
}

std::wstring Utf8ToWide(const std::string& value) {
    if (value.empty()) return L"";
    int required = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
    if (required <= 0) return L"";
    std::wstring out((size_t)required - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, out.data(), required);
    return out;
}

bool HttpJsonRequest(const wchar_t* method, const wchar_t* path, const std::string& body, const std::string& bearerToken, std::string& responseStr) {
    responseStr.clear();

    HINTERNET hSession = WinHttpOpen(LICENSE_USER_AGENT, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;

    HINTERNET hConnect = WinHttpConnect(hSession, LICENSE_API_HOST, INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return false;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, method, path, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    std::wstring headers = skCrypt(L"Content-Type: application/json\r\nUser-Agent: GZ-Account-Loader\r\n");
    if (!bearerToken.empty()) {
        headers += skCrypt(L"Authorization: Bearer ");
        headers += Utf8ToWide(bearerToken);
        headers += skCrypt(L"\r\n");
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

bool HttpJsonPost(const wchar_t* path, const nlohmann::json& requestJson, const std::string& bearerToken, std::string& responseStr) {
    return HttpJsonRequest(skCrypt(L"POST"), path, requestJson.dump(), bearerToken, responseStr);
}

bool HttpJsonPut(const wchar_t* path, const nlohmann::json& requestJson, const std::string& bearerToken, std::string& responseStr) {
    return HttpJsonRequest(skCrypt(L"PUT"), path, requestJson.dump(), bearerToken, responseStr);
}

bool HttpJsonGet(const wchar_t* path, const std::string& bearerToken, std::string& responseStr) {
    return HttpJsonRequest(skCrypt(L"GET"), path, skCrypt(""), bearerToken, responseStr);
}

struct LoaderSessionFile {
    std::string token;
    std::string username;
    std::string key;
    std::string config_code;
};

LoaderSessionFile LoadLoaderSessionFile() {
    LoaderSessionFile session;
    std::ifstream in(AppPaths::LoaderSessionPath());
    if (!in.is_open()) {
        in.clear();
        in.open(skCrypt("loader_session.json"));
    }
    if (!in.is_open()) return session;

    nlohmann::json json = nlohmann::json::parse(in, nullptr, false);
    if (json.is_discarded() || !json.is_object()) return session;

    session.token = JsonStringValue(json, skCrypt("token"));
    session.username = JsonStringValue(json, skCrypt("username"));
    session.key = JsonStringValue(json, skCrypt("key"));
    session.config_code = JsonStringValue(json, skCrypt("config_code"));
    return session;
}

void SaveLoaderSessionFile() {
    nlohmann::json json;
    json["token"] = global_account_token;
    json["username"] = global_account_username;
    json["key"] = global_active_key;
    json["config_code"] = global_config_code;

    std::ofstream out(AppPaths::LoaderSessionPath(), std::ios::trunc);
    if (out.is_open()) {
        out << json.dump(2);
    }
}

void ClearLoaderSessionFile() {
    DeleteFileA(AppPaths::LoaderSessionPath().c_str());
    DeleteFileA(AppPaths::KeyPath().c_str());
    DeleteFileA(skCrypt("loader_session.json"));
    DeleteFileA(skCrypt("key.txt"));
}

bool ApplyExpiryFromJson(const nlohmann::json& json, const char* expiryField = "expiry", const char* timestampField = "timestamp") {
    std::string expiry = JsonStringValue(json, expiryField);
    if (expiry.empty()) expiry = JsonStringValue(json, skCrypt("expires_at"));
    std::string server_time = JsonStringValue(json, timestampField);
    if (server_time.empty()) server_time = JsonStringValue(json, skCrypt("server_time"));
    if (server_time.empty()) {
        server_time = JsonStringValue(json, skCrypt("timestamp"));
    }

    if (expiry.empty()) return false;
    time_t exp_t = ParseISO8601(expiry);
    time_t srv_t = server_time.empty() ? time(nullptr) : ParseISO8601(server_time);
    if (exp_t > srv_t) {
        g_remaining_seconds = (uint64_t)(exp_t - srv_t);
        g_last_tick_count = GetTickCount64();
        g_expiry_time = exp_t;
        g_expiry_str = expiry;
        return true;
    }

    g_remaining_seconds = 0;
    return false;
}

bool ValidateSignedLicensePayload(
    const nlohmann::json& responseJson,
    const std::string& expectedAction,
    const std::string& expectedStatus,
    const std::string& expectedKey,
    const std::string& expectedHwid,
    const std::string& expectedNonce,
    std::string& signedStatus,
    std::string& signedExpiry,
    std::string& signedTimestamp,
    std::string& error
) {
    std::string signedPayload = JsonStringValue(responseJson, "signed_payload");
    std::string serverSignature = JsonStringValue(responseJson, "signature");

    if (signedPayload.empty() || serverSignature.empty()) {
        error = skCrypt("SIGNED_PAYLOAD_MISSING");
        return false;
    }

    if (!Crypto::VerifyRSASignature(RSA_PUBLIC_KEY, signedPayload, serverSignature)) {
        error = skCrypt("SIGNATURE_INVALID");
        return false;
    }

    nlohmann::json payloadJson = nlohmann::json::parse(signedPayload, nullptr, false);
    if (payloadJson.is_discarded() || !payloadJson.is_object()) {
        error = skCrypt("SIGNED_PAYLOAD_INVALID_JSON");
        return false;
    }

    if (JsonInt64Value(payloadJson, "v") != LICENSE_SIGNATURE_VERSION) {
        error = skCrypt("SIGNED_PAYLOAD_VERSION_MISMATCH");
        return false;
    }

    signedStatus = JsonStringValue(payloadJson, "status");
    signedExpiry = JsonStringValue(payloadJson, "expiry");
    signedTimestamp = JsonStringValue(payloadJson, "timestamp");

    if (JsonStringValue(payloadJson, "action") != expectedAction ||
        signedStatus != expectedStatus ||
        JsonStringValue(payloadJson, "key") != expectedKey ||
        JsonStringValue(payloadJson, "hwid") != expectedHwid ||
        JsonStringValue(payloadJson, "nonce") != expectedNonce) {
        error = skCrypt("SIGNED_PAYLOAD_FIELD_MISMATCH");
        return false;
    }

    if (JsonStringValue(responseJson, "status") != signedStatus) {
        error = skCrypt("RESPONSE_STATUS_MISMATCH");
        return false;
    }

    long long issuedAt = JsonInt64Value(payloadJson, "issued_at");
    long long now = (long long)time(nullptr);
    long long age = now > issuedAt ? now - issuedAt : issuedAt - now;
    if (issuedAt <= 0 || age > LICENSE_SIGNATURE_MAX_AGE_SECONDS) {
        error = skCrypt("SIGNED_PAYLOAD_EXPIRED");
        return false;
    }

    return true;
}

bool DoAPIRequest(const std::string& key, const std::string& hwid, bool silent) {
    global_license_error = "";
    const wchar_t* endpointPath = silent ? LOADER_HEARTBEAT_PATH : LOADER_KEY_ACTIVATE_PATH;

    if (global_account_token.empty()) {
        global_license_error = g_is_vietnamese ? skCrypt("CHUA DANG NHAP TAI KHOAN.") : skCrypt("ACCOUNT IS NOT LOGGED IN.");
        return false;
    }

    std::string nonce = GenerateSecureNonce();
    nlohmann::json requestJson;
    if (!key.empty()) requestJson["key"] = key;
    requestJson["hwid"] = hwid;
    requestJson["nonce"] = nonce;
    requestJson["signature_version"] = LICENSE_SIGNATURE_VERSION;
    std::string responseStr = "";
    HttpJsonPost(endpointPath, requestJson, global_account_token, responseStr);

    if (responseStr.empty()) {
        if (!silent) {
            SetConsoleColor(12);
            std::string err = g_is_vietnamese ? skCrypt("Loi ket noi may chu. Vui long thu lai.") : skCrypt("Connection error. Please try again.");
            std::cout << "[-] " << err << std::endl;
            global_license_error = err;
            SetConsoleColor(7);
        }
        return false;
    }

    nlohmann::json responseJson = nlohmann::json::parse(responseStr, nullptr, false);
    if (responseJson.is_discarded() || !responseJson.is_object()) {
        if (!silent) {
            SetConsoleColor(12);
            std::string err = g_is_vietnamese ? skCrypt("Phan hoi may chu khong hop le.") : skCrypt("Invalid server response.");
            std::cout << "[-] " << err << std::endl;
            global_license_error = err;
            SetConsoleColor(7);
        }
        return false;
    }

    std::string status = JsonStringValue(responseJson, "status");
    const std::string expectedAction = silent ? skCrypt("heartbeat") : skCrypt("activate");
    const std::string expectedStatus = silent ? skCrypt("OK") : skCrypt("ACTIVATED");
    if (status == expectedStatus) {
        if (!silent) {
            // std::cout << "[DEBUG] Status: " << status << std::endl;
            // std::cout << "[DEBUG] HWID: " << hwid << std::endl;
            // std::cout << "[DEBUG] Response: " << responseStr << std::endl;
        }

        // --- BƯỚC KHÓA BẢO MẬT CUỐI CÙNG (RSA SIGNATURE VERIFICATION) ---
        
        // 1. Tìm signature từ server
        std::string signedStatus;
        std::string signedExpiry;
        std::string signedTimestamp;
        std::string verifyError;
        
        // 2. Tìm timestamp từ server để khớp hash
        bool signedPayloadOk = ValidateSignedLicensePayload(
            responseJson,
            expectedAction,
            expectedStatus,
            key,
            hwid,
            nonce,
            signedStatus,
            signedExpiry,
            signedTimestamp,
            verifyError);

        // 3. XÁC MINH CHỮ KÝ RSA + TÍNH TOÀN VẸN (BINARY INTEGRITY)
        // Dữ liệu cần xác minh: "key:hwid:timestamp:nonce"
        
        
        // if (!silent) std::cout << "[DEBUG] DataToVerify: " << dataToVerify << std::endl;

        // Thêm hash binary vào chuỗi xác thực để server kiểm tra (nếu backend hỗ trợ)
        // Hiện tại ta chỉ Verify signature của server.
        if (!signedPayloadOk) {
            if (!silent) {
                SetConsoleColor(12);
                std::string err = g_is_vietnamese ? skCrypt("Xac thuc toan ven that bai! SERVER GIA.") : skCrypt("Integrity check failed! FAKE SERVER.");
                err += skCrypt(" ");
                err += verifyError;
                std::cout << "[-] " << err << std::endl;
                global_license_error = err;
                SetConsoleColor(7);
            }
            g_session_token = 0;
            return false;
        }

        std::string responseConfigCode = JsonStringValue(responseJson, skCrypt("config_code"));
        if (!responseConfigCode.empty()) global_config_code = responseConfigCode;

        if (!signedExpiry.empty()) {
            nlohmann::json expiryJson;
            expiryJson["expiry"] = signedExpiry;
            expiryJson["timestamp"] = signedTimestamp;
            ApplyExpiryFromJson(expiryJson, skCrypt("expiry"), skCrypt("timestamp"));
        }

        // Kiểm tra hash định kỳ so với giá trị nhúng ở server (Phần này Admin cần cấu hình ở Worker)
        // if (serverExpectedHash != localBinaryHash) { ... return false; }

        if (silent) {
            return true;
        }

        if (!silent) {
            std::string expiry = signedExpiry;
            std::string server_time = signedTimestamp;

            if (!expiry.empty() && !server_time.empty()) {
                time_t exp_t = ParseISO8601(expiry);
                time_t srv_t = ParseISO8601(server_time);
                
                if (exp_t > srv_t) {
                    g_remaining_seconds = (uint64_t)(exp_t - srv_t);
                    g_last_tick_count = GetTickCount64();
                    g_expiry_time = exp_t; // Vẫn lưu để hiển thị nếu cần
                } else {
                    g_remaining_seconds = 0;
                }
            }

            SetConsoleColor(10);
            if (g_is_vietnamese) {
                std::cout << skCrypt("[+] Xac thuc Key thanh cong!\n");
                if (!expiry.empty()) std::cout << skCrypt("[*] Ngay het han: ") << expiry << std::endl;
            } else {
                std::cout << skCrypt("[+] License authenticated successfully!\n");
                if (!expiry.empty()) std::cout << skCrypt("[*] Expiry Date: ") << expiry << std::endl;
            }
            SetConsoleColor(7);
        }

        // SECURITY: Store a unique session token using CSPRNG
        {
            HCRYPTPROV hCryptProv = 0;
            if (CryptAcquireContextA(&hCryptProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
                CryptGenRandom(hCryptProv, sizeof(g_session_token), (BYTE*)&g_session_token);
                CryptReleaseContext(hCryptProv, 0);
            } else {
                // Fallback: still better than predictable XOR
                LARGE_INTEGER perf;
                QueryPerformanceCounter(&perf);
                g_session_token = (uint32_t)(perf.QuadPart ^ (uintptr_t)&hCryptProv ^ GetCurrentProcessId());
            }
        }
        return true;
    } else if (responseStr.find(skCrypt("SESSION_REPLACED")) != std::string::npos ||
               responseStr.find(skCrypt("SESSION_HWID_MISMATCH")) != std::string::npos) {
        std::string err = g_is_vietnamese ? skCrypt("PHIEN DANG NHAP DA BI THAY THE. Vui long dang nhap lai.") : skCrypt("SESSION REPLACED. Please log in again.");
        global_license_error = err;
        global_account_token.clear();
        global_account_username.clear();
        global_account_role.clear();
        global_config_code.clear();
        ClearActiveEntitlementState();
        ClearLoaderSessionFile();
        if (!silent) {
            SetConsoleColor(12);
            std::cout << "[-] " << err << std::endl;
            SetConsoleColor(7);
        }
        return false;
    } else if (responseStr.find(skCrypt("HWID_MISMATCH")) != std::string::npos) {
        std::string err = g_is_vietnamese ? skCrypt("SAI HWID: Tai khoan dang khoa tren may khac.") : skCrypt("HWID MISMATCH: Account is active on another PC.");
        global_license_error = err;
        if (!silent) {
            SetConsoleColor(12);
            std::cout << "[-] " << err << std::endl;
            SetConsoleColor(7);
        }
        return false;
    } else if (responseStr.find(skCrypt("EXPIRED")) != std::string::npos) {
        std::string err = g_is_vietnamese ? skCrypt("KEY HET HAN: Vui long gia han.") : skCrypt("LICENSE EXPIRED: Please renew.");
        global_license_error = err;
        if (!silent) {
            SetConsoleColor(12);
            std::cout << "[-] " << err << std::endl;
            SetConsoleColor(7);
        }
        return false;
    } else if (responseStr.find(skCrypt("BANNED")) != std::string::npos) {
        std::string err = g_is_vietnamese ? skCrypt("BAN BI BAN: Thiet bi nay bi cam vinh vien!") : skCrypt("YOU ARE BANNED: Permanently restricted!");
        global_license_error = err;
        if (!silent) {
            SetConsoleColor(12);
            std::cout << "[-] " << err << std::endl;
            SetConsoleColor(7);
        }
        return false;
    } else if (responseStr.find(skCrypt("INVALID_API_KEY")) != std::string::npos) {
        std::string err = g_is_vietnamese ? skCrypt("API SECRET SAI: Vui long cap nhat loader.") : skCrypt("INVALID API SECRET: Please update loader.");
        global_license_error = err;
        if (!silent) {
            SetConsoleColor(12);
            std::cout << "[-] " << err << std::endl;
            SetConsoleColor(7);
        }
        return false;
    } else if (responseStr.find(skCrypt("AUTH_NOT_CONFIGURED")) != std::string::npos) {
        std::string err = g_is_vietnamese ? skCrypt("SERVER CHUA CAU HINH API SECRET.") : skCrypt("SERVER API SECRET IS NOT CONFIGURED.");
        global_license_error = err;
        if (!silent) {
            SetConsoleColor(12);
            std::cout << "[-] " << err << std::endl;
            SetConsoleColor(7);
        }
        return false;
    } else if (responseStr.find(skCrypt("STATUS_REVOKED")) != std::string::npos) {
        std::string err = g_is_vietnamese ? skCrypt("KEY DA BI THU HOI.") : skCrypt("LICENSE REVOKED.");
        global_license_error = err;
        if (!silent) {
            SetConsoleColor(12);
            std::cout << "[-] " << err << std::endl;
            SetConsoleColor(7);
        }
        return false;
    } else if (responseStr.find(skCrypt("REPLAY_DETECTED")) != std::string::npos || responseStr.find(skCrypt("INVALID_NONCE")) != std::string::npos) {
        std::string err = g_is_vietnamese ? skCrypt("LOI BAO MAT NONCE: Vui long khoi dong lai loader.") : skCrypt("NONCE SECURITY ERROR: Please restart loader.");
        global_license_error = err;
        if (!silent) {
            SetConsoleColor(12);
            std::cout << "[-] " << err << std::endl;
            SetConsoleColor(7);
        }
        return false;
    } else if (responseStr.find(skCrypt("PRODUCT_LOCKED")) != std::string::npos) {
        std::string err = g_is_vietnamese ? skCrypt("SAN PHAM HOAC GOI KEY DANG BI KHOA.") : skCrypt("PRODUCT OR KEY TYPE IS LOCKED.");
        global_license_error = err;
        if (!silent) {
            SetConsoleColor(12);
            std::cout << "[-] " << err << std::endl;
            SetConsoleColor(7);
        }
        return false;
    }

    if (!silent) {
        SetConsoleColor(12);
        std::cout << (g_is_vietnamese ? skCrypt("[-] LOI SERVER: ") : skCrypt("[-] SERVER ERROR: ")) << responseStr << std::endl;
        SetConsoleColor(7);
    }
    return false;
}

bool DownloadLoaderConfig() {
    if (global_account_token.empty()) return false;

    std::string responseStr;
    if (!HttpJsonGet(LOADER_CONFIG_PATH, global_account_token, responseStr)) return false;

    nlohmann::json responseJson = nlohmann::json::parse(responseStr, nullptr, false);
    if (responseJson.is_discarded() || !responseJson.is_object()) return false;

    std::string code = JsonStringValue(responseJson, skCrypt("config_code"));
    if (!code.empty()) global_config_code = code;

    auto it = responseJson.find(skCrypt("config"));
    if (it == responseJson.end() || !it->is_object() || it->empty()) return true;

    std::ofstream out(AppPaths::SettingsConfigPath(), std::ios::trunc);
    if (!out.is_open()) return false;
    out << it->dump(2);
    return true;
}

bool UploadLoaderConfig() {
    if (global_account_token.empty()) return false;

    std::ifstream in(AppPaths::SettingsConfigPath());
    if (!in.is_open()) {
        in.clear();
        in.open(skCrypt("dataMacro\\Config\\settings.json"));
    }
    if (!in.is_open()) return false;

    nlohmann::json configJson = nlohmann::json::parse(in, nullptr, false);
    if (configJson.is_discarded() || !configJson.is_object()) return false;

    nlohmann::json requestJson;
    requestJson["config"] = configJson;

    std::string responseStr;
    if (!HttpJsonPut(LOADER_CONFIG_PATH, requestJson, global_account_token, responseStr)) return false;

    nlohmann::json responseJson = nlohmann::json::parse(responseStr, nullptr, false);
    if (responseJson.is_discarded() || !responseJson.is_object()) return false;

    std::string code = JsonStringValue(responseJson, skCrypt("config_code"));
    if (!code.empty()) global_config_code = code;
    SaveLoaderSessionFile();
    return JsonStringValue(responseJson, skCrypt("status")) == skCrypt("OK");
}

void UploadActiveLoaderConfigAsync() {
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)[](LPVOID) {
        UploadLoaderConfig();
        return (DWORD)0;
    }, NULL, 0, NULL);
}

bool ImportLoaderConfigCode(const std::string& code) {
    if (global_account_token.empty() || code.empty()) return false;

    nlohmann::json requestJson;
    requestJson["code"] = code;

    std::string responseStr;
    if (!HttpJsonPost(LOADER_CONFIG_IMPORT_PATH, requestJson, global_account_token, responseStr)) return false;

    nlohmann::json responseJson = nlohmann::json::parse(responseStr, nullptr, false);
    if (responseJson.is_discarded() || !responseJson.is_object()) return false;
    if (JsonStringValue(responseJson, skCrypt("status")) != skCrypt("IMPORTED")) return false;

    std::string ownCode = JsonStringValue(responseJson, skCrypt("config_code"));
    if (!ownCode.empty()) global_config_code = ownCode;

    auto it = responseJson.find(skCrypt("config"));
    if (it != responseJson.end() && it->is_object()) {
        std::ofstream out(AppPaths::SettingsConfigPath(), std::ios::trunc);
        if (out.is_open()) out << it->dump(2);
    }

    SaveLoaderSessionFile();
    return true;
}

bool ParseAuthSessionResponse(const std::string& responseStr, bool allowNoActiveKey) {
    nlohmann::json responseJson = nlohmann::json::parse(responseStr, nullptr, false);
    if (responseJson.is_discarded() || !responseJson.is_object()) return false;

    if (responseJson.contains(skCrypt("error"))) {
        global_license_error = JsonStringValue(responseJson, skCrypt("error"));
        return false;
    }

    std::string topStatus = JsonStringValue(responseJson, skCrypt("status"));
    if (topStatus == skCrypt("SESSION_REPLACED") ||
        topStatus == skCrypt("SESSION_HWID_MISMATCH") ||
        topStatus == skCrypt("BANNED")) {
        global_license_error = topStatus;
        global_account_token.clear();
        global_account_username.clear();
        global_account_role.clear();
        global_config_code.clear();
        ClearActiveEntitlementState();
        ClearLoaderSessionFile();
        return false;
    }

    if (!topStatus.empty() &&
        topStatus != skCrypt("OK") &&
        topStatus != skCrypt("REGISTERED") &&
        topStatus != skCrypt("ACTIVATED")) {
        global_license_error = topStatus;
        return false;
    }

    std::string token = JsonStringValue(responseJson, skCrypt("token"));
    if (!token.empty()) global_account_token = token;

    bool hasUser = false;
    auto userIt = responseJson.find(skCrypt("user"));
    if (userIt != responseJson.end() && userIt->is_object()) {
        hasUser = true;
        std::string username = JsonStringValue(*userIt, skCrypt("username"));
        if (!username.empty()) global_account_username = username;
        std::string role = JsonStringValue(*userIt, skCrypt("role"));
        if (!role.empty()) global_account_role = role;
    }

    std::string configCode = JsonStringValue(responseJson, skCrypt("config_code"));
    if (!configCode.empty()) global_config_code = configCode;

    auto entitlementIt = responseJson.find(skCrypt("entitlement"));
    if (entitlementIt != responseJson.end() && entitlementIt->is_object()) {
        std::string key = JsonStringValue(*entitlementIt, skCrypt("key"));
        bool active = (*entitlementIt).value(skCrypt("active"), false);
        if (active && !key.empty() && ApplyExpiryFromJson(*entitlementIt, skCrypt("expires_at"), skCrypt("timestamp"))) {
            global_active_key = key;
        } else {
            ClearActiveEntitlementState();
        }
        return HasActiveLoaderEntitlement() || allowNoActiveKey;
    }

    ClearActiveEntitlementState();
    return allowNoActiveKey && (hasUser || !token.empty());
}

bool TryResumeLoaderAccount(const std::string& hwid) {
    LoaderSessionFile session = LoadLoaderSessionFile();
    if (session.token.empty()) return false;

    global_account_token = session.token;
    global_account_username = session.username;
    global_active_key = session.key;
    global_config_code = session.config_code;

    std::wstring path = std::wstring(LOADER_ME_PATH) + skCrypt(L"?hwid=") + Utf8ToWide(hwid);
    std::string responseStr;
    if (!HttpJsonGet(path.c_str(), global_account_token, responseStr)) {
        global_account_token.clear();
        return false;
    }

    if (!ParseAuthSessionResponse(responseStr, true)) {
        global_account_token.clear();
        return false;
    }

    if (!global_active_key.empty() && DoAPIRequest(global_active_key, hwid, true)) {
        return true;
    }

    return !global_account_token.empty();
}

bool PromptLoaderAccountLogin(const std::string& hwid) {
    SetConsoleColor(11);
    std::cout << skCrypt("\n[1] Dang nhap tai khoan\n[2] Tao tai khoan moi\n> ");
    SetConsoleColor(15);

    std::string choice;
    std::getline(std::cin, choice);
    bool registering = choice == skCrypt("2");

    std::string username;
    std::string password;
    std::cout << skCrypt("Tai khoan: ");
    std::getline(std::cin, username);
    std::cout << skCrypt("Mat khau: ");
    std::getline(std::cin, password);

    nlohmann::json requestJson;
    requestJson["username"] = username;
    requestJson["password"] = password;
    requestJson["hwid"] = hwid;

    std::string responseStr;
    if (!HttpJsonPost(registering ? LOADER_REGISTER_PATH : LOADER_LOGIN_PATH, requestJson, skCrypt(""), responseStr)) {
        global_license_error = g_is_vietnamese ? skCrypt("Khong ket noi duoc may chu tai khoan.") : skCrypt("Cannot connect to account server.");
        return false;
    }

    if (!ParseAuthSessionResponse(responseStr, true) || global_account_token.empty()) {
        global_license_error = responseStr;
        return false;
    }

    SaveLoaderSessionFile();
    return true;
}

bool AuthenticateLicense() {
    std::string hwid = GetHWID();
    
    // Silent resume if session exists
    if (TryResumeLoaderAccount(hwid)) {
        if (!global_active_key.empty() && DoAPIRequest(global_active_key, hwid, true)) {
            DownloadLoaderConfig();
            SaveLoaderSessionFile();
            return true;
        }
    }

    // Don't block. Let the UI handle credentials.
    // We return true because the 'Loader' (main process) needs to keep running to show the overlay.
    return true; 
}

void LicenseHeartbeatLoop() {
    std::string hwid = GetHWID();
    uint64_t last_net_check_tick = GetTickCount64();

    while (true) {
        Sleep(1000 * 5); // Kiểm tra đếm ngược cực chuẩn mỗi 5 giây
        
        uint64_t current_tick = GetTickCount64();
        uint64_t elapsed_seconds = (current_tick - g_last_tick_count) / 1000;

        // 1. Kiểm tra đếm ngược Real-time (Relative from Server)
        if (g_remaining_seconds > 0 && elapsed_seconds >= g_remaining_seconds) {
            SetConsoleColor(12);
            std::cout << (g_is_vietnamese ? skCrypt("\n\n[!!!] THOI GIAN SU DUNG DA HET (Real-time Expiry).\n") : skCrypt("\n\n[!!!] SUBSCRIPTION ENDED (Real-time Expiry).\n"));
            SetConsoleColor(7);
            Sleep(5000);
            exit(1);
        }

        // 2. Kiểm tra mạng định kỳ (đề phòng Admin thu hồi Key) - 10 phút/lần
        if ((current_tick - last_net_check_tick) >= (600 * 1000)) {
            if (!global_active_key.empty()) {
                bool heartbeatOk = DoAPIRequest(global_active_key, hwid, true);
                if (!heartbeatOk && !global_license_error.empty()) {
                    SetConsoleColor(12);
                    std::cout << (g_is_vietnamese ? skCrypt("\n\n[!!!] LICENSE KHONG CON HOP LE: ") : skCrypt("\n\n[!!!] LICENSE IS NO LONGER VALID: ")) << global_license_error << std::endl;
                    SetConsoleColor(7);
                    Sleep(5000);
                    exit(1);
                }
            }
            last_net_check_tick = current_tick;
        }
    }
}

void SelfDestruct() {
    // SECURITY: Delete configuration files first
    DeleteFileA(AppPaths::KeyPath().c_str());
    DeleteFileA(AppPaths::LoaderSessionPath().c_str());
    DeleteFileA(skCrypt("key.txt"));
    DeleteFileA(skCrypt("loader_session.json"));

    char szModuleName[MAX_PATH];
    if (GetModuleFileNameA(NULL, szModuleName, MAX_PATH) == 0) return;

    // Use a delayed command to delete the binary after the process has exited
    // Also try to delete other potential traces in the folder if needed
    std::string cmd = std::string(skCrypt("cmd.exe /C timeout /T 2 /NOBREAK > Nul & del /f /q \"")) + szModuleName + skCrypt("\"");
    
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

void SelfRename() {
    char szOriginalPath[MAX_PATH];
    if (GetModuleFileNameA(NULL, szOriginalPath, MAX_PATH) == 0) return;

    std::string dir = szOriginalPath;
    size_t lastBackslash = dir.find_last_of(skCrypt("\\"));
    if (lastBackslash != std::string::npos) {
        dir = dir.substr(0, lastBackslash + 1);
    }

    std::string newName = "";
    static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < 12; i++) {
        newName += alphabet[rand() % 26];
    }
    std::string newPath = dir + newName + skCrypt(".exe");

    // Windows allows renaming the file even if it's currently running (but not deleting it)
    MoveFileExA(szOriginalPath, newPath.c_str(), MOVEFILE_REPLACE_EXISTING);
}

// Task 2.4/2.5: Anti-Debug, Anti-Dump, & Anti-Crack Checks

bool CheckHardwareBreakpoints() {
    CONTEXT ctx = { 0 };
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    HANDLE hThread = GetCurrentThread();
    if (GetThreadContext(hThread, &ctx)) {
        if (ctx.Dr0 != 0 || ctx.Dr1 != 0 || ctx.Dr2 != 0 || ctx.Dr3 != 0) {
            return true;
        }
    }
    return false;
}

bool CheckBlacklistedProcesses() {
    const wchar_t* blacklisted[] = { skCrypt(L"ida64.exe"), skCrypt(L"x64dbg.exe"), skCrypt(L"integrity_monitorengine-x86_64.exe"), skCrypt(L"Fiddler.exe"), skCrypt(L"wireshark.exe"), skCrypt(L"HTTPDebuggerSvc.exe"), skCrypt(L"ida.exe"), skCrypt(L"x32dbg.exe") };
    for (const auto& proc : blacklisted) {
        if (telemetryHyperProcess::FindProcessIdByName(proc) != 0) {
            return true;
        }
    }
    return false;
}

bool SecurityCheck() {
    // 1. Basic Debugger Check
    if (IsDebuggerPresent()) return false;
    
    BOOL debugged = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &debugged);
    if (debugged) return false;

    // 2. Advanced Debugger Check (NtQueryInformationProcess)
    typedef NTSTATUS (NTAPI *pNtQueryInformationProcess)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
    auto ntqip = (pNtQueryInformationProcess)GetProcAddress(GetModuleHandleA(skCrypt("ntdll.dll")), skCrypt("NtQueryInformationProcess"));
    if (ntqip) {
        DWORD_PTR debugPort = 0;
        if (ntqip(GetCurrentProcess(), (PROCESSINFOCLASS)7, &debugPort, sizeof(debugPort), NULL) == 0 && debugPort != 0)
            return false;

        HANDLE debugObject = NULL;
        if (ntqip(GetCurrentProcess(), (PROCESSINFOCLASS)0x1E, &debugObject, sizeof(debugObject), NULL) == 0 && debugObject != NULL)
            return false;

        DWORD debugFlags = 1;
        if (ntqip(GetCurrentProcess(), (PROCESSINFOCLASS)0x1F, &debugFlags, sizeof(debugFlags), NULL) == 0 && debugFlags == 0)
            return false;
    }

    // 3. (Bảo mật) Quét Window đã bị xóa bỏ để tránh trigger Anti-
    // Auto-Discovery nay dùng SharedMemory.
    if (CheckHardwareBreakpoints()) return false;
    if (CheckBlacklistedProcesses()) return false;
    
    return true;
}



bool IsUserAdmin() {
    BOOL bIsAdmin = FALSE;
    PSID AdministratorsGroup = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup)) {
        CheckTokenMembership(NULL, AdministratorsGroup, &bIsAdmin);
        FreeSid(AdministratorsGroup);
    }
    return bIsAdmin;
}

bool IsProcessElevated(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) return false;
    HANDLE hToken = NULL;
    bool elevated = false;
    if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD size = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &size)) {
            elevated = elevation.TokenIsElevated;
        }
        CloseHandle(hToken);
    }
    CloseHandle(hProcess);
    return elevated;
}

// Hàm tìm Discord đặc quyền bị loại bỏ do Node chuyển sang cơ chế Passive Discovery.

VisualizationBridgeHost ResolvePassiveVisualizationHost() {
    return VisualizationBridgeDiscovery::ResolveHost();
}

bool QueryProcessData(uint32_t pid, query_process_data_packet *output) {
  return telemetryMemory::QueryProcessData(pid, output);
}

struct ProcessPickDebugInfo {
  int matched_name = 0;
  int query_ok = 0;
  int query_fail = 0;
  int invalid_ctx = 0;
  int invalid_mz = 0;
  int open_fail = 0;
  DWORD selected_pid = 0;
  uint64_t selected_base = 0;
  uint64_t selected_cr3 = 0;
  uint64_t selected_cycles = 0;
  uint64_t selected_working_set = 0;
  uint64_t selected_private_usage = 0;
};

void CleanUpEFITraces() {
#ifdef _DEBUG
  std::cout << skCrypt("[BOOT-SAFETY] EFI cleanup is disabled in telemetry to avoid boot/BIOS side effects.") << std::endl;
#endif
  return;

#if 0
  // [STEALTH] Dọn dẹp dấu vết trên EFI ngay lập tức
  std::string drive = skCrypt("Z:");
  
  // Mount partition EFI vào ổ Z:
  system((skCrypt("mountvol ") + drive + skCrypt(" /S >nul 2>&1")).c_str());
  Sleep(1000); // Đợi mount ổn định

  // 1. Hồi phục file Bootloader gốc của Windows (Sử dụng MoveFileExA để ghi đè an toàn)
  auto restore_boot = [&](const std::string& orig_rel, const std::string& current_rel) {
      std::string current_path = drive + current_rel;
      std::string backup_path = drive + orig_rel;
      for (auto& c : current_path) if (c == '/') c = '\\';
      for (auto& c : backup_path) if (c == '/') c = '\\';

      if (GetFileAttributesA(backup_path.c_str()) != INVALID_FILE_ATTRIBUTES) {
          // Gỡ bảo vệ file giả triệt để
          SetFileAttributesA(current_path.c_str(), FILE_ATTRIBUTE_NORMAL);
          
          // Thử khôi phục bằng MoveFileEx với cờ REPLACE_EXISTING
          if (MoveFileExA(backup_path.c_str(), current_path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
              // Gán lại thuộc tính hệ thống ẩn (+s +h +r) để trùng khớp 100% với file chuẩn Microsoft
              SetFileAttributesA(current_path.c_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY);
          } else {
              // Nếu MoveFileEx thất bại, dùng lệnh system attrib để cứu vãn
              system((skCrypt("attrib -s -h -r \"") + current_path + skCrypt("\" >nul 2>&1")).c_str());
              DeleteFileA(current_path.c_str());
              MoveFileA(backup_path.c_str(), current_path.c_str());
              SetFileAttributesA(current_path.c_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY);
          }
      }
  };

  restore_boot(skCrypt("/EFI/Microsoft/Boot/bootmgfw.original.efi"), skCrypt("/EFI/Microsoft/Boot/bootmgfw.efi"));
  restore_boot(skCrypt("/EFI/Boot/bootx64.original.efi"), skCrypt("/EFI/Boot/bootx64.efi"));

  // 2. Xóa sạch các artifact còn lại
  std::vector<std::string> efi_files = {
      skCrypt("/EFI/Microsoft/Boot/uefi-boot.efi"),
      skCrypt("/EFI/Microsoft/Boot/loader.exe"),
      skCrypt("/EFI/Microsoft/Boot/update_utility.exe"),
      skCrypt("/EFI/Microsoft/Boot/sys_utility.exe"),
      skCrypt("/EFI/Microsoft/Boot/load-hyper-reV.txt"),
      skCrypt("/EFI/Microsoft/Boot/system_auth.dll"),
      skCrypt("/EFI/Microsoft/Boot/hyperv-attachment.dll"),
      skCrypt("/EFI/Microsoft/Boot/bootmgfw.original.efi"),
      skCrypt("/EFI/Boot/bootx64.original.efi")
  };

  for (const auto& file : efi_files) {
      std::string path = drive + file;
      for (auto& c : path) if (c == '/') c = '\\';
      SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_NORMAL);
      DeleteFileA(path.c_str());
  }

  // Unmount partition EFI
  system((skCrypt("mountvol ") + drive + skCrypt(" /D >nul 2>&1")).c_str());
#endif
}

static ProcessPickDebugInfo g_pid_debug = {};

int main() {
  ProcessSingleInstance::Guard singleInstance;
  if (!singleInstance.Acquire()) {
#ifdef _DEBUG
      std::cout << "[!] Could not stop the previous tool instance.\n";
#else
      MessageBoxA(NULL, 
          skCrypt("Could not stop the previous tool instance.\nKhong the tat tien trinh cu."),
          skCrypt("GZ-telemetry - ALREADY RUNNING"), MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL | MB_TOPMOST);
#endif
      return 0;
  }

  EnsureLoaderConsole();
  protec::scan_detection_time = 1000;
  srand((unsigned int)GetTickCount64());
  protec::start_protect(false);
  SelfRename();

  if (!SecurityCheck()) {
      SelfDestruct();
      return 0;
  }

  // [STEALTH] Check Admin & Wipe EFI traces immediately upon startup
  if (!IsUserAdmin()) {
      SetConsoleColor(12);
      std::cout << (g_is_vietnamese ? skCrypt("[-] Vui long chay bang quyen QTV (Run as Admin) - Error: 0x5") : skCrypt("[-] Missing permission (Error: 0x5)")) << std::endl;
      DebugPause();
      SelfDestruct();
      return 1;
  }
  CleanUpEFITraces();
  
  SelectLanguage();

  SetConsoleColor(7);

  if (!AuthenticateLicense()) {
      DebugPause();
      SelfDestruct();
      return 1;
  }

  SetConsoleColor(7);

  TypewriterPrint("\n[", 10, 8);
  TypewriterPrint("1", 10, 11);
  TypewriterPrint("] ", 10, 8);
  TypewriterPrint(g_is_vietnamese ? skCrypt("Checking system...") : skCrypt("Checking system..."), 30, 7);
  std::cout << "\n";
  
  /*
  // KIỂM TRA BẢO MẬT: Bắt buộc bật Ảo hóa (VT-x / SVM) từ BIOS
  if (!IsProcessorFeaturePresent(PF_VIRT_FIRMWARE_ENABLED)) {
      SetConsoleColor(12);
      std::cout << (g_is_vietnamese ? 
          skCrypt("[-] LOI NGHIEM TRONG: Ban chua bat Ao hoa (VT-x / SVM) trong BIOS!\n[-] Vui long vao BIOS (F2/F10/Del) de bat 'Virtualization Technology' truoc khi dung diagnostic_node.\n") : 
          skCrypt("[-] CRITICAL ERROR: Hardware Virtualization (VT-x / SVM) is disabled in BIOS!\n[-] Please enable 'Virtualization Technology' in BIOS before using.\n"));
      SetConsoleColor(7);
      DebugPause();
      return 1;
  }
  */

  // (Removed redundant second license check)


  // Start background heartbeat to monitor expiration
  CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)[](LPVOID) {
      LicenseHeartbeatLoop();
      return (DWORD)0;
  }, NULL, 0, NULL);

  srand((unsigned int)GetTickCount64());
  
  char rand_title[16] = { 0 };
  for (int i = 0; i < 15; i++) rand_title[i] = (rand() % 26) + 'a';
  SetConsoleTitleA(rand_title);


  TypewriterPrint("\n[", 10, 8);
  TypewriterPrint("2", 10, 11);
  TypewriterPrint("] ", 10, 8);
  TypewriterPrint(g_is_vietnamese ? skCrypt("Checking hyper connection...") : skCrypt("Checking hyper connection..."), 30, 7);
  std::cout << "\n";

  query_process_data_packet test_packet = {};
  NTSTATUS status = (telemetryMemory::InitializeHyperInterface())
      ? 0
      : static_cast<NTSTATUS>(0xC0000001L);
  if (status >= 0) {
      std::cout << (g_is_vietnamese ? skCrypt("[+] Hypervisor connection established!") : skCrypt("[+] Hypervisor connection established!")) << std::endl;
  }
  
  if (status < 0) {
#ifdef _DEBUG
    SetConsoleColor(12);
    std::cout << (g_is_vietnamese ? skCrypt("[-] Khong thể giao tiep voi hypervisor!\n") : skCrypt("[-] Failed to communicate with hypervisor!\n"));
    SetConsoleColor(7);
#else
    MessageBoxA(NULL, 
        skCrypt("CRITICAL ERROR: Hypervisor connection failed!\nLoi nghiem trong: Khong the ket noi Hypervisor!\n\nPlease run 'GZ-Loader' as Administrator first, then reopen this tool.\nVui long chay 'GZ-Loader' bang quyen Admin truoc, sau do mo lai Tool nay."), 
        skCrypt("GZ-telemetry - HYPERVISOR ERROR"), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_TOPMOST);
#endif
    DebugPause();
    SelfDestruct();
    return 1;
  }
  
  // Discord Requirement Check bị loại bỏ vì Discovery Bridge hoạt động trong âm thầm
  // và không còn bắt buộc phải có Discord (có thể dùng Shared Registry Host khác).

  Sleep(500);
  // (Removed VMouse KDU Loading as Logitech G-Hub is used instead)

  
  TypewriterPrint("\n[", 10, 8);
  TypewriterPrint("3", 10, 11);
  TypewriterPrint("] ", 10, 8);
  TypewriterPrint(g_is_vietnamese ? skCrypt("Waiting for telemetry (TslGame.exe)...") : skCrypt("Waiting for telemetry (TslGame.exe)..."), 30, 14);
  
  DWORD pid = 0;
  int wait_pid_count = 0;
  constexpr ULONGLONG kPidSettleDelayMs = 3000;
  ULONGLONG first_candidate_tick = 0;
  DWORD pending_pid = 0;
  static wchar_t game_name[] = { 'T','s','l','G','a','m','e','.','e','x','e',0 };
  while (!pid) {
    wait_pid_count++;
    query_process_data_packet candidate_data = {};
    if (!telemetryHyperProcess::QueryProcessData(0, &candidate_data)) {
      first_candidate_tick = 0;
      pending_pid = 0;
      std::cout << skCrypt(".");
      if (wait_pid_count % 5 == 0) {
        std::cout << (g_is_vietnamese ? skCrypt("\n[DEBUG][PID] try=") : skCrypt("\n[DEBUG][PID] try=")) << wait_pid_count << (g_is_vietnamese ? skCrypt(" (Ghost-walking cho TslGame...)") : skCrypt(" (Ghost-walking for TslGame...)")) << std::endl;
      }
      Sleep(500 + (rand() % 200));
      continue;
    }

    DWORD candidate_pid = (DWORD)candidate_data.process_id;
    ULONGLONG candidate_cr3 = candidate_data.cr3;

    if (pending_pid != candidate_pid) {
      pending_pid = candidate_pid;
      first_candidate_tick = GetTickCount64();
      std::cout << skCrypt("\n[DEBUG][PID] candidate_detected=") << pending_pid
                << skCrypt(" initial_settling_ms=") << kPidSettleDelayMs
                << std::endl;
    }

    const ULONGLONG elapsed = GetTickCount64() - first_candidate_tick;
    
    // CR3 Check: If the process has a valid page directory and we've met the minimum threshold (e.g. 500ms), 
    // or if the full timer expired, we are good to go.
    if (candidate_cr3 != 0 && elapsed > 1000) {
        std::cout << skCrypt("[+] CR3 verified! Active at: ") << std::hex << candidate_cr3 << std::dec << std::endl;
        pid = candidate_pid;
        break;
    }

    if (elapsed < kPidSettleDelayMs) {
      Sleep(500);
      continue;
    }

    pid = candidate_pid;
  }
  
  if (!telemetryMemory::AttachToGameStealthily(pid)) {
      SetConsoleColor(12);
      std::cout << skCrypt("\n[-] Critical Communication Error (Stealth Auth Fail)!") << std::endl;
      DebugPause();
      SelfDestruct();
      return 1;
  }
  uint64_t base = telemetryMemory::g_BaseAddress;

  TypewriterPrint("\n[", 10, 8);
  TypewriterPrint("4", 10, 11);
  TypewriterPrint("] ", 10, 8);
  TypewriterPrint(g_is_vietnamese ? skCrypt("Connecting to game engine via hyper...") : skCrypt("Connecting to game engine via hyper..."), 30, 7);
  std::cout << "\n";
  SetConsoleColor(14);
  int sync_count = 0;
  ULONGLONG engine_wait_started = GetTickCount64();
  constexpr ULONGLONG kEngineWaitDiagnosticMs = 30000;
  while (!telemetryContext::Initialize(pid, base)) {
      sync_count++;
      std::cout << (g_is_vietnamese ? skCrypt("\r[*] Dang cho game san sang qua hyper [") : skCrypt("\r[*] Waiting for hyper-backed game state [")) << sync_count << skCrypt("]...   ") << std::flush;
      if (sync_count % 5 == 0) {
          query_process_data_packet live_data = {};
          const bool live_ok = telemetryHyperProcess::QueryProcessData(pid, &live_data);
          if (live_ok && live_data.process_id != 0) {
              const uint64_t live_base = reinterpret_cast<uint64_t>(live_data.base_address);
              std::cout << skCrypt("\n[DEBUG][ENGINE] status=") << telemetryContext::GetLastInitializeStatus()
                        << skCrypt(" pid=") << live_data.process_id
                        << skCrypt(" cr3=0x") << std::hex << live_data.cr3
                        << skCrypt(" base=0x") << live_base << std::dec << std::endl;
              if (live_data.cr3 != telemetryMemory::g_ProcessCr3 || live_base != telemetryMemory::g_BaseAddress) {
                  telemetryMemory::g_ProcessCr3 = live_data.cr3;
                  telemetryMemory::g_BaseAddress = live_base;
                  telemetryMemory::g_LastRefreshTime = 0;
                  base = live_base;
                  std::cout << skCrypt("[DEBUG][ENGINE] refreshed process context") << std::endl;
              }
          } else {
              std::cout << skCrypt("\n[DEBUG][ENGINE] status=") << telemetryContext::GetLastInitializeStatus()
                        << skCrypt(" process query failed; returning to PID wait may be needed") << std::endl;
          }
      }
      if (GetTickCount64() - engine_wait_started > kEngineWaitDiagnosticMs) {
          std::cout << skCrypt("\n[DEBUG][ENGINE] still waiting after 30s; last_status=")
                    << telemetryContext::GetLastInitializeStatus()
                    << skCrypt(". If the game is still loading, enter lobby/training; otherwise restart the game process.") << std::endl;
          engine_wait_started = GetTickCount64();
      }
      Sleep(1000 + (rand() % 200));
  }
  std::cout << (g_is_vietnamese ? skCrypt("\n[+] Ket noi hyper thanh cong, hay vao game de trai nghiem!") : skCrypt("\n[+] Hyper connection ready!")) << std::endl;
  SetConsoleColor(7);
  
    MacroEngine::Initialize();

    // EptMouse::InstallMouseEptHook(); // [TEMPORARY OFF]

    // [STEALTH] Robust Visualization Bridge Attachment (Loop until ready)
    VisualizationBridgeHost visualization_bridge = {};
    bool menu_initialized = false;
    int init_retry_count = 0;
    
    std::cout << skCrypt("[*] Synchronizing Visualization Bridge (waiting for Discord Overlay to be fully ready)...\n");

    while (!menu_initialized && init_retry_count < 60) {
        visualization_bridge = ResolvePassiveVisualizationHost();
        
        if (visualization_bridge.hwnd) {
            if (g_Menu.Initialize(visualization_bridge)) {
                menu_initialized = true;
                break;
            } else {
                std::cout << skCrypt("[-] Menu initialization failed (DirectX might not be ready on target HWND). Retrying...\n");
            }
        }

        init_retry_count++;
        Sleep(1000);
    }

    if (!menu_initialized) {
        SetConsoleColor(12);
        std::cout << skCrypt("[-] CRITICAL: Visualization bridge could not be synchronized after 60 seconds.\n");
        std::cout << skCrypt("[-] Please ensure Discord Overlay is enabled for TslGame.exe and try again.\n");
        SetConsoleColor(7);
#ifndef _DEBUG
        MessageBoxA(NULL,
            skCrypt("Visualization bridge host could not be resolved or initialized.\nEnsure Discord is open and Overlay is enabled for ."),
            skCrypt("GZ-telemetry - VISUALIZATION ERROR"), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_TOPMOST);
#endif
        DebugPause();
        SelfDestruct();
        return 1;
    }

    // [DKOM] Hide our own process ONLY AFTER menu is attached
    DWORD our_pid = GetCurrentProcessId();
    std::cout << skCrypt("[DKOM] Our PID: ") << our_pid << std::endl;
    uint64_t our_eprocess = telemetryHyperProcess::GetEProcessAddress(our_pid);
    std::cout << skCrypt("[DKOM] EPROCESS Address: 0x") << std::hex << our_eprocess << std::dec << std::endl;
    if (our_eprocess) {
        bool result = telemetryHyperProcess::UnlinkProcessDKOM(our_eprocess);
        std::cout << skCrypt("[DKOM] UnlinkProcess result: ") << (result ? "SUCCESS" : "FAILED") << std::endl;
    } else {
        std::cout << skCrypt("[DKOM] FAILED: Could not find our EPROCESS!") << std::endl;
    }
    // [ANTI-DUMP] Safe Erasing DOS headers (Now compatible with DirectX)
    protec::erase_pe_header();
    
#ifdef _DEBUG
    std::cout << (g_is_vietnamese ? skCrypt("\n[+] He thong da san sang! Hay mo game telemetry.") : skCrypt("\n[+] System Ready! Please open telemetry.")) << std::endl;
    std::cout << (g_is_vietnamese ? skCrypt("[+] Bam [F5] de Dong/Mo Menu | Bam [F11] de Tat Tool") : skCrypt("[+] F5: Menu | F11: Clean Exit")) << std::endl;
    std::cout << skCrypt("[DEBUG] Running in development mode. Console will remain open.\n");
#else
    // RELEASE MODE: Use Bilingual System Modal (Top 1) MessageBox
    MessageBoxA(NULL, 
        skCrypt("System Ready! Please open telemetry.\nHe thong da san sang! Hay mo game telemetry.\n\nPress [F5] for Menu. / Bam [F5] de Dong/Mo Menu."), 
        skCrypt("GZ-telemetry - System Ready"), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL | MB_SETFOREGROUND | MB_TOPMOST);
#endif

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)[](LPVOID) {
        while (true) {
            GameData.Keyboard.UpdateKeys();
            MacroEngine::Update();
            Sleep(1);
        }
        return (DWORD)0;
    }, NULL, 0, NULL);

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)[](LPVOID) {
      while (true) {
        telemetryContext::UpdateGameData();
        // [ULTRA STEALTH] Random delay between 100ms and 300ms
        telemetryMemory::StealthSleep(100 + (rand() % 201)); 
      }
      return (DWORD)0;
    }, NULL, 0, NULL);
    
    while (!AppShutdown::IsRequested()) {
        if ((GetAsyncKeyState(VK_END) & 1) || (GetAsyncKeyState(VK_F11) & 1)) {
            AppShutdown::Request();
            break;
        }

        // Message Pump (Bắt buộc phải có để cửa sổ Fallback không bị "Not Responding" và nhận được Input)
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                AppShutdown::Request();
                break;
            }
        }

        if (AppShutdown::IsRequested()) break;
        g_Menu.RenderFrame();
        if (AppShutdown::IsRequested()) break;
        // [LATENCY JITTER] Dynamic render sleep
        telemetryMemory::StealthSleep(g_Menu.render_sleep);
    }
    
    g_Menu.Shutdown();
    telemetryDecrypt::Cleanup();
    // LogitechMouse::Shutdown(); // No longer used — kernel mouse only
    SelfDestruct();
    return 0;
}
