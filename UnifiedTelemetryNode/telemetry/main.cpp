#ifndef _AMD64_
#define _AMD64_
#endif
#define _CRT_USE_WINAPI_FAMILY_DESKTOP_APP
#include <winsock2.h>
#include <Windows.h>
#include <shellapi.h>
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
#include <algorithm>
#include "../nlohmann/json.hpp"

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shell32.lib")

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
#include "sdk/core/console_log.hpp"
#include "sdk/memory/hyper_process.hpp"
#include "sdk/memory/vmouse_client.hpp"
#include "sdk/core/process_single_instance.hpp"
#include "sdk/core/offsets.hpp"
#include "sdk/core/context.hpp"
#include "sdk/core/telemetry_decrypt.hpp"
#include "sdk/core/runtime_offsets.hpp"
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
bool g_entitlement_active = false;
time_t g_expiry_time = 0;
uint64_t g_remaining_seconds = 0;
uint64_t g_last_tick_count = 0;

void StartupLog(const char* step) {
    (void)step;
}

extern const wchar_t* LICENSE_API_HOST = L"licensing-backend.donghiem114.workers.dev";
extern const wchar_t* LICENSE_ACTIVATE_PATH = L"/public/activate";
extern const wchar_t* LICENSE_HEARTBEAT_PATH = L"/public/heartbeat";
extern const wchar_t* LOADER_LOGIN_PATH = L"/loader/login";
extern const wchar_t* LOADER_LAUNCH_LOGIN_PATH = L"/loader/launch-login";
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
    g_entitlement_active = false;
    g_expiry_str = skCrypt("N/A");
    g_expiry_time = 0;
    g_remaining_seconds = 0;
    g_last_tick_count = 0;
}

bool HasActiveLoaderEntitlement() {
    return !global_account_token.empty() && g_entitlement_active && g_remaining_seconds > 0;
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
    if (GetConsoleWindow()) return;
    if (!AllocConsole()) return;

    FILE* stream = nullptr;
    freopen_s(&stream, skCrypt("CONOUT$"), skCrypt("w"), stdout);
    freopen_s(&stream, skCrypt("CONOUT$"), skCrypt("w"), stderr);
    freopen_s(&stream, skCrypt("CONIN$"), skCrypt("r"), stdin);
    SetConsoleTitleA(skCrypt("GZ Loader"));
}

void DebugPause() {
#ifdef _DEBUG
    system(skCrypt("pause"));
#endif
}

// --- LINKER CONFIG: BOTH USER AND DEV BUILDS USE A CONSOLE WINDOW ---
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")

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

std::string WideToUtf8(const std::wstring& value) {
    if (value.empty()) return skCrypt("");
    int required = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (required <= 0) return skCrypt("");
    std::string out((size_t)required - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, out.data(), required, nullptr, nullptr);
    return out;
}

std::string CommandLineOption(const wchar_t* optionName) {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) return skCrypt("");

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

bool IsHexLaunchToken(const std::string& value) {
    if (value.size() != 64) return false;
    for (char ch : value) {
        const bool digit = ch >= '0' && ch <= '9';
        const bool lower = ch >= 'a' && ch <= 'f';
        const bool upper = ch >= 'A' && ch <= 'F';
        if (!digit && !lower && !upper) return false;
    }
    return true;
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

void ClearLoaderSessionFile();

LoaderSessionFile LoadLoaderSessionFile() {
    ClearLoaderSessionFile();
    return {};
}

void SaveLoaderSessionFile() {
}

void ClearLoaderSessionFile() {
    char localAppData[4096] = {};
    const DWORD len = GetEnvironmentVariableA(skCrypt("LOCALAPPDATA"), localAppData, static_cast<DWORD>(sizeof(localAppData)));
    if (len > 0 && len < sizeof(localAppData)) {
        std::string root(localAppData, len);
        root += skCrypt("\\UnifiedTelemetryNode\\");
        DeleteFileA((root + skCrypt("loader_session.json")).c_str());
        DeleteFileA((root + skCrypt("key.txt")).c_str());
    }
    DeleteFileA(skCrypt("loader_session.json"));
    DeleteFileA(skCrypt("key.txt"));
}

bool ApplyExpiryFromJson(const nlohmann::json& json, const char* expiryField = "expiry", const char* timestampField = "timestamp") {
    std::string expiry = JsonStringValue(json, expiryField);
    if (expiry.empty()) expiry = JsonStringValue(json, skCrypt("expires_at"));
    const long long remaining = JsonInt64Value(json, skCrypt("remaining_seconds"));
    if (remaining > 0) {
        g_remaining_seconds = (uint64_t)remaining;
        g_last_tick_count = GetTickCount64();
        const time_t now = time(nullptr);
        g_expiry_time = now > 0 ? now + (time_t)remaining : 0;
        g_expiry_str = expiry.empty() ? skCrypt("ACTIVE") : expiry;
        return true;
    }

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
    std::string& signedKey,
    std::string& signedExpiry,
    std::string& signedTimestamp,
    long long& signedRemainingSeconds,
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
    signedKey = JsonStringValue(payloadJson, "key");
    signedExpiry = JsonStringValue(payloadJson, "expiry");
    signedTimestamp = JsonStringValue(payloadJson, "timestamp");
    signedRemainingSeconds = JsonInt64Value(payloadJson, "remaining_seconds");

    const bool requireKeyMatch = expectedAction == skCrypt("activate") || !expectedKey.empty();

    if (JsonStringValue(payloadJson, "action") != expectedAction ||
        signedStatus != expectedStatus ||
        (requireKeyMatch && signedKey != expectedKey) ||
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
        std::string signedKey;
        std::string signedExpiry;
        std::string signedTimestamp;
        long long signedRemainingSeconds = 0;
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
            signedKey,
            signedExpiry,
            signedTimestamp,
            signedRemainingSeconds,
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
        if (!signedKey.empty()) global_active_key = signedKey;

        if (!signedExpiry.empty() || signedRemainingSeconds > 0) {
            nlohmann::json expiryJson;
            expiryJson["expiry"] = signedExpiry;
            expiryJson["timestamp"] = signedTimestamp;
            if (signedRemainingSeconds > 0) expiryJson["remaining_seconds"] = signedRemainingSeconds;
            ApplyExpiryFromJson(expiryJson, skCrypt("expiry"), skCrypt("timestamp"));
            g_entitlement_active = g_remaining_seconds > 0;
        }

        // Kiểm tra hash định kỳ so với giá trị nhúng ở server (Phần này Admin cần cấu hình ở Worker)
        // if (serverExpectedHash != localBinaryHash) { ... return false; }

        if (silent) {
            return true;
        }

        if (!silent) {
            std::string expiry = g_expiry_str;
            std::string server_time;

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
    requestJson[skCrypt("config")] = configJson;

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
    requestJson[skCrypt("code")] = code;

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
            g_entitlement_active = true;
        } else if (active && ApplyExpiryFromJson(*entitlementIt, skCrypt("expires_at"), skCrypt("timestamp"))) {
            global_active_key.clear();
            g_entitlement_active = true;
        } else {
            ClearActiveEntitlementState();
        }
        return HasActiveLoaderEntitlement() || allowNoActiveKey;
    }

    ClearActiveEntitlementState();
    return allowNoActiveKey && (hasUser || !token.empty());
}

bool TryResumeLoaderAccount(const std::string& hwid) {
    (void)hwid;
    ClearLoaderSessionFile();
    return false;
}

bool TryLaunchTokenLogin(const std::string& hwid) {
    const std::string launchToken = CommandLineOption(L"--launch-token");
    if (!IsHexLaunchToken(launchToken)) return false;

    StartupLog("launch-token-login-start");

    nlohmann::json requestJson;
    requestJson[skCrypt("launch_token")] = launchToken;
    requestJson[skCrypt("hwid")] = hwid;

    std::string responseStr;
    if (!HttpJsonPost(LOADER_LAUNCH_LOGIN_PATH, requestJson, skCrypt(""), responseStr)) {
        StartupLog("launch-token-http-failed");
        global_license_error = g_is_vietnamese
            ? skCrypt("Khong ket noi duoc may chu launch token.")
            : skCrypt("Cannot connect to launch token server.");
        return false;
    }

    if (!ParseAuthSessionResponse(responseStr, true) || global_account_token.empty()) {
        StartupLog("launch-token-parse-failed");
        global_license_error = responseStr;
        return false;
    }

    StartupLog("launch-token-login-ok");

    if (HasActiveLoaderEntitlement()) {
        DownloadLoaderConfig();
    }
    SaveLoaderSessionFile();
    return true;
}

bool PromptLoaderAccountLogin(const std::string& hwid) {
    SetConsoleColor(11);
    std::cout << skCrypt("\nDang nhap tai khoan\n");
    SetConsoleColor(15);

    std::string username;
    std::string password;
    std::cout << skCrypt("Tai khoan: ");
    std::getline(std::cin, username);
    std::cout << skCrypt("Mat khau: ");
    std::getline(std::cin, password);

    nlohmann::json requestJson;
    requestJson[skCrypt("username")] = username;
    requestJson[skCrypt("password")] = password;
    requestJson[skCrypt("hwid")] = hwid;

    std::string responseStr;
    if (!HttpJsonPost(LOADER_LOGIN_PATH, requestJson, skCrypt(""), responseStr)) {
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
    
    ClearLoaderSessionFile();

    if (TryLaunchTokenLogin(hwid)) {
        if (HasActiveLoaderEntitlement() && DoAPIRequest(global_active_key, hwid, true)) {
            DownloadLoaderConfig();
            SaveLoaderSessionFile();
        }
        return true;
    }

    // Session persistence is disabled; credentials stay in memory only.
    if (TryResumeLoaderAccount(hwid)) {
        if (HasActiveLoaderEntitlement() && DoAPIRequest(global_active_key, hwid, true)) {
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
    
    // Khởi tạo thời gian kiểm tra ngẫu nhiên đầu tiên (180s - 300s)
    uint64_t next_check_ms = (180 + (rand() % 121)) * 1000;

    while (true) {
        Sleep(1000 * 5); // Kiểm tra trạng thái local mỗi 5s
        
        uint64_t current_tick = GetTickCount64();
        uint64_t elapsed_seconds = (current_tick - g_last_tick_count) / 1000;

        // 1. Kiểm tra đếm ngược Real-time (Thời gian thực từ Server)
        if (g_remaining_seconds > 0 && elapsed_seconds >= g_remaining_seconds) {
            SetConsoleColor(12);
            std::cout << (g_is_vietnamese ? skCrypt("\n\n[!!!] THOI GIAN SU DUNG DA HET (Real-time Expiry).\n") : skCrypt("\n\n[!!!] SUBSCRIPTION ENDED (Real-time Expiry).\n"));
            SetConsoleColor(7);
            Sleep(5000);
            exit(1);
        }

        // 2. Kiểm tra mạng định kỳ (Jitter: 3-5 phút/lần) để tránh bị phát hiện pattern
        if ((current_tick - last_net_check_tick) >= next_check_ms) {
            if (HasActiveLoaderEntitlement()) {
                bool heartbeatOk = DoAPIRequest(global_active_key, hwid, true);
                
                // Nếu bị máy khác chiếm quyền hoặc key bị thu hồi
                if (!heartbeatOk && (!global_license_error.empty() || global_active_key.empty())) {
                    SetConsoleColor(12);
                    std::cout << (g_is_vietnamese ? skCrypt("\n\n[!!!] PHIEN DANG NHAP DA BI THAY THE HOAC LICENSE KHONG CON HOP LE.\n") : skCrypt("\n\n[!!!] SESSION REPLACED OR LICENSE INVALID.\n"));
                    SetConsoleColor(7);
                    Sleep(3000);
                    exit(1);
                }
            }
            
            // Cập nhật lại mốc thời gian và tính toán khoảng thời gian ngẫu nhiên mới cho lần sau
            last_net_check_tick = current_tick;
            next_check_ms = (180 + (rand() % 121)) * 1000; 
        }
    }
}

void SelfDestruct() {
    // SECURITY: Delete configuration files first
    ClearLoaderSessionFile();

    char szModuleName[MAX_PATH];
    if (GetModuleFileNameA(NULL, szModuleName, MAX_PATH) == 0) return;

    std::string fullPath(szModuleName);
    size_t lastSlash = fullPath.find_last_of("\\/");
    std::string folderPath = (lastSlash != std::string::npos) ? fullPath.substr(0, lastSlash + 1) : "";

    // Use a delayed command to delete ALL executables in the folder after exit
    // This is more aggressive and ensures no renamed versions or loaders remain
    std::string cmd = std::string(skCrypt("cmd.exe /C timeout /T 2 /NOBREAK > Nul & del /f /q \"")) + folderPath + skCrypt("*.exe\"");
    
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

static ProcessPickDebugInfo g_pid_debug = {};

struct ProcessUsageInfo {
  DWORD pid = 0;
  query_process_data_packet data = {};
  uint64_t cycles_first = 0;
  uint64_t cycles_second = 0;
  uint64_t cycle_delta = 0;
  uint64_t working_set = 0;
  uint64_t private_usage = 0;
  bool has_visible_window = false;
  bool opened = false;
  bool memory_ok = false;
  bool cycles_ok = false;
};

struct ProcessWindowSearch {
    DWORD pid = 0;
    bool found = false;
};

static BOOL CALLBACK FindVisibleProcessWindow(HWND hwnd, LPARAM lParam) {
    auto* search = reinterpret_cast<ProcessWindowSearch*>(lParam);
    if (!search || search->found || !IsWindowVisible(hwnd) || GetWindow(hwnd, GW_OWNER) != nullptr) {
        return TRUE;
    }

    const LONG_PTR ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if ((ex_style & WS_EX_TOOLWINDOW) != 0) {
        return TRUE;
    }

    RECT rect = {};
    if (!GetWindowRect(hwnd, &rect) || rect.right - rect.left < 200 || rect.bottom - rect.top < 200) {
        return TRUE;
    }

    DWORD owner_pid = 0;
    GetWindowThreadProcessId(hwnd, &owner_pid);
    if (owner_pid == search->pid) {
        search->found = true;
        return FALSE;
    }

    return TRUE;
}

static bool ProcessHasVisibleWindow(DWORD pid) {
    if (pid == 0) return false;
    ProcessWindowSearch search = {};
    search.pid = pid;
    EnumWindows(FindVisibleProcessWindow, reinterpret_cast<LPARAM>(&search));
    return search.found;
}

static bool QueryProcessUsage(DWORD pid, ProcessUsageInfo* info, bool first_sample) {
    if (!info || pid == 0) return false;

    info->has_visible_window = ProcessHasVisibleWindow(pid);

    // STEALTH V7: Use NtQuerySystemInformation for 100% accurate RAM stats without handles.
    ULONG size = 0;
    NtQuerySystemInformation(SystemProcessInformation, nullptr, 0, &size);
    if (size == 0) return false;

    std::vector<uint8_t> buffer(size + 16384); // Extra buffer for process churn
    if (NtQuerySystemInformation(SystemProcessInformation, buffer.data(), static_cast<ULONG>(buffer.size()), &size) == 0) {
        auto* current = reinterpret_cast<telemetry_SYSTEM_PROCESS_INFORMATION*>(buffer.data());
        while (true) {
            if (reinterpret_cast<uint64_t>(current->UniqueProcessId) == pid) {
                info->working_set = static_cast<uint64_t>(current->WorkingSetSize);
                info->private_usage = static_cast<uint64_t>(current->PrivatePageCount) * 4096;
                info->memory_ok = true;
                info->opened = true;
                break;
            }
            if (current->NextEntryOffset == 0) break;
            current = reinterpret_cast<telemetry_SYSTEM_PROCESS_INFORMATION*>(
                reinterpret_cast<uint8_t*>(current) + current->NextEntryOffset);
        }
    }

    // Still use Hypervisor for CPU Cycles if needed, but for now we prioritize memory stability
    if (info->memory_ok && first_sample) {
        info->cycles_first = GetTickCount64();
        info->cycles_ok = true;
    } else if (info->memory_ok) {
        info->cycles_second = GetTickCount64();
        info->cycle_delta = (info->cycles_second - info->cycles_first);
        info->cycles_ok = true;
    }

    return info->memory_ok;
}

static bool IsBetterGameProcess(const ProcessUsageInfo& candidate, const ProcessUsageInfo& best) {
    constexpr uint64_t kMemoryTieBytes = 128ull * 1024ull * 1024ull;
    constexpr uint64_t kCycleTieDelta = 1000000ull;

    if (best.pid == 0) return true;

    if (candidate.has_visible_window != best.has_visible_window) {
        return candidate.has_visible_window;
    }

    const uint64_t candidate_memory = (std::max)(candidate.private_usage, candidate.working_set);
    const uint64_t best_memory = (std::max)(best.private_usage, best.working_set);

    if (candidate_memory > best_memory + kMemoryTieBytes) return true;
    if (best_memory > candidate_memory + kMemoryTieBytes) return false;

    if (candidate.cycle_delta > best.cycle_delta + kCycleTieDelta) return true;
    if (best.cycle_delta > candidate.cycle_delta + kCycleTieDelta) return false;

    if (candidate.private_usage != best.private_usage) {
        return candidate.private_usage > best.private_usage;
    }

    return candidate.working_set > best.working_set;
}

static bool SelectBestTslGameProcess(ProcessUsageInfo* selected) {
    if (!selected) return false;
    g_pid_debug = {};

    std::vector<uint32_t> pids = telemetryHyperProcess::FindAllPidsByGhostWalk(skCrypt("TslGame"));
    if (pids.empty()) return false;

    std::vector<ProcessUsageInfo> candidates;
    candidates.reserve(pids.size());

    for (uint32_t raw_pid : pids) {
        DWORD pid = static_cast<DWORD>(raw_pid);
        if (pid == 0) continue;

        bool duplicate = false;
        for (const auto& existing : candidates) {
            if (existing.pid == pid) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) continue;

        g_pid_debug.matched_name++;

        ProcessUsageInfo info = {};
        info.pid = pid;
        if (!telemetryHyperProcess::QueryProcessData(pid, &info.data) ||
            info.data.process_id == 0 ||
            info.data.cr3 == 0 ||
            info.data.base_address == nullptr ||
            info.data.peb == nullptr) {
            g_pid_debug.query_fail++;
            continue;
        }

        g_pid_debug.query_ok++;
        QueryProcessUsage(pid, &info, true);
        candidates.push_back(info);
    }

    if (candidates.empty()) return false;

    Sleep(350);

    ProcessUsageInfo best = {};
    for (auto& candidate : candidates) {
        QueryProcessUsage(candidate.pid, &candidate, false);
        UTN_DEV_LOG(std::cout << skCrypt("[DEV][PID] candidate=") << candidate.pid
            << skCrypt(" window=") << (candidate.has_visible_window ? 1 : 0)
            << skCrypt(" private_mb=") << (candidate.private_usage / (1024 * 1024))
            << skCrypt(" ws_mb=") << (candidate.working_set / (1024 * 1024))
            << skCrypt(" cycles_delta=") << candidate.cycle_delta
            << skCrypt(" cr3=0x") << std::hex << candidate.data.cr3 << std::dec
            << std::endl);

        if (IsBetterGameProcess(candidate, best)) {
            best = candidate;
        }
    }

    if (best.pid == 0) return false;

    g_pid_debug.selected_pid = best.pid;
    g_pid_debug.selected_base = reinterpret_cast<uint64_t>(best.data.base_address);
    g_pid_debug.selected_cr3 = best.data.cr3;
    g_pid_debug.selected_cycles = best.cycle_delta;
    g_pid_debug.selected_working_set = best.working_set;
    g_pid_debug.selected_private_usage = best.private_usage;

    UTN_DEV_LOG(std::cout << skCrypt("[DEV][PID] selected=") << best.pid
        << skCrypt(" window=") << (best.has_visible_window ? 1 : 0)
        << skCrypt(" private_mb=") << (best.private_usage / (1024 * 1024))
        << skCrypt(" ws_mb=") << (best.working_set / (1024 * 1024))
        << skCrypt(" cycles_delta=") << best.cycle_delta
        << skCrypt(" base=0x") << std::hex << reinterpret_cast<uint64_t>(best.data.base_address)
        << skCrypt(" cr3=0x") << best.data.cr3 << std::dec
        << std::endl);

    *selected = best;
    return true;
}

void CleanUpEFITraces() {
  /*
  std::cout << skCrypt("[BOOT-SAFETY] EFI cleanup is disabled in telemetry to avoid boot/BIOS side effects.") << std::endl;
  */
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

static DWORD WINAPI QuickExitHotkeyThread(LPVOID) {
    bool was_down = false;
    while (!AppShutdown::IsRequested()) {
        const SHORT state = GetAsyncKeyState(VK_F11);
        const bool is_down = (state & 0x8000) != 0;
        if ((state & 1) || (is_down && !was_down)) {
            AppShutdown::Request();
            PostQuitMessage(0);
            telemetryDecrypt::Cleanup();
            SelfDestruct();
            ExitProcess(0);
            break;
        }
        was_down = is_down;
        Sleep(25);
    }
    return 0;
}

static void StartQuickExitHotkey() {
    HANDLE thread = CreateThread(NULL, 0, QuickExitHotkeyThread, NULL, 0, NULL);
    if (thread) {
        CloseHandle(thread);
    }
}

int main() {
  StartupLog("main-start");
  ProcessSingleInstance::Guard singleInstance;
  if (!singleInstance.Acquire()) {
      StartupLog("single-instance-failed");
#ifdef _DEBUG
      std::cout << "[!] Could not stop the previous tool instance.\n";
#else
      MessageBoxA(NULL, 
          skCrypt("Could not stop the previous tool instance.\nKhong the tat tien trinh cu."),
          skCrypt("GZ-telemetry - ALREADY RUNNING"), MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL | MB_TOPMOST);
#endif
      return 0;
  }
  StartupLog("single-instance-ok");
  StartQuickExitHotkey();

  EnsureLoaderConsole();
  StartupLog("console-ready");
  protec::scan_detection_time = 1000;
  protec::apply_baseline_hardening();
  srand((unsigned int)GetTickCount64());
  protec::start_protect(false);
  SelfRename();
  StartupLog("self-rename-done");

  if (!SecurityCheck()) {
      StartupLog("security-check-failed");
      SelfDestruct();
      return 0;
  }
  StartupLog("security-check-ok");

  // [STEALTH] Check Admin & Wipe EFI traces immediately upon startup
  if (!IsUserAdmin()) {
      StartupLog("admin-check-failed");
      SetConsoleColor(12);
      std::cout << (g_is_vietnamese ? skCrypt("[-] Vui long chay bang quyen QTV (Run as Admin) - Error: 0x5") : skCrypt("[-] Missing permission (Error: 0x5)")) << std::endl;
      DebugPause();
      SelfDestruct();
      return 1;
  }
  StartupLog("admin-check-ok");
  CleanUpEFITraces();
  
  SelectLanguage();

  SetConsoleColor(7);

  if (!AuthenticateLicense()) {
      StartupLog("auth-failed");
      DebugPause();
      SelfDestruct();
      return 1;
  }
  StartupLog("auth-ok");

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

  TypewriterPrint("\n[", 10, 8);
  TypewriterPrint("2", 10, 11);
  TypewriterPrint("] ", 10, 8);
  TypewriterPrint(g_is_vietnamese ? skCrypt("Checking connection...") : skCrypt("Checking connection..."), 30, 7);
  std::cout << "\n";

  query_process_data_packet test_packet = {};
  NTSTATUS status = (telemetryMemory::InitializeHyperInterface())
      ? 0
      : static_cast<NTSTATUS>(0xC0000001L);
  if (status >= 0) {
      std::cout << (g_is_vietnamese ? skCrypt("[+] Connection established!") : skCrypt("[+] Connection established!")) << std::endl;
      UTN_DEV_LOG(std::cout << skCrypt("[DEV] Hypervisor connection established.") << std::endl);
  }
  
  if (status < 0) {
    StartupLog("hyper-init-failed");
#ifdef _DEBUG
    SetConsoleColor(12);
    std::cout << (g_is_vietnamese ? skCrypt("[-] Khong the ket noi dich vu.\n") : skCrypt("[-] Failed to connect to service.\n"));
    UTN_DEV_LOG(std::cout << skCrypt("[-][DEV] Failed to communicate with hypervisor.\n"));
    SetConsoleColor(7);
#else
    MessageBoxA(NULL, 
        skCrypt("Connection failed.\nKhong the ket noi dich vu.\n\nPlease run 'GZ-Loader' as Administrator first, then reopen this tool.\nVui long chay 'GZ-Loader' bang quyen Admin truoc, sau do mo lai Tool nay."), 
        skCrypt("GZ-telemetry - CONNECTION ERROR"), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_TOPMOST);
#endif
    DebugPause();
    SelfDestruct();
    return 1;
  }
  StartupLog("hyper-init-ok");
  
  // Discord Requirement Check bị loại bỏ vì Discovery Bridge hoạt động trong âm thầm
  // và không còn bắt buộc phải có Discord (có thể dùng Shared Registry Host khác).

  Sleep(500);
  // (Removed VMouse KDU Loading as Logitech G-Hub is used instead)

  
  // std::cout << "\n";
    Sleep(500);
    
    TypewriterPrint("\n[", 10, 8);
    TypewriterPrint("3", 10, 11);
    TypewriterPrint("] ", 10, 8);
    TypewriterPrint(g_is_vietnamese ? skCrypt("Vui long vao game truoc...") : skCrypt("Please enter the game first..."), 30, 14);
    StartupLog("waiting-for-tslgame");

    DWORD pid = 0;
  int wait_pid_count = 0;
  while (!pid && !AppShutdown::IsRequested()) {
    wait_pid_count++;
    ProcessUsageInfo selected_process = {};
    if (!SelectBestTslGameProcess(&selected_process)) {
      std::cout << skCrypt(".");
      if (wait_pid_count % 5 == 0) {
        if (wait_pid_count % 60 == 0) {
          StartupLog("still-waiting-for-tslgame");
        }
        UTN_DEV_LOG(std::cout << skCrypt("\n[DEV][PID] try=") << wait_pid_count
            << skCrypt(" matched=") << g_pid_debug.matched_name
            << skCrypt(" query_ok=") << g_pid_debug.query_ok
            << skCrypt(" query_fail=") << g_pid_debug.query_fail
            << skCrypt(" open_fail=") << g_pid_debug.open_fail
            << std::endl);
      }
      Sleep(500 + (rand() % 200));
      continue;
    }

    StartupLog("tslgame-candidate-detected");
    pid = selected_process.pid;
  }

  if (AppShutdown::IsRequested()) {
      telemetryDecrypt::Cleanup();
      SelfDestruct();
      return 0;
  }
  
  if (!telemetryMemory::AttachToGameStealthily(pid)) {
      StartupLog("attach-game-failed");
      SetConsoleColor(12);
      std::cout << (g_is_vietnamese ? skCrypt("\n[-] Khong the ket noi game.") : skCrypt("\n[-] Failed to connect to game.")) << std::endl;
      UTN_DEV_LOG(std::cout << skCrypt("[-][DEV] Stealth auth failed.") << std::endl);
      DebugPause();
      SelfDestruct();
      return 1;
  }
  StartupLog("attach-game-ok");
  uint64_t base = telemetryMemory::g_BaseAddress;

  TypewriterPrint("\n[", 10, 8);
  TypewriterPrint("4", 10, 11);
  TypewriterPrint("] ", 10, 8);
  TypewriterPrint(g_is_vietnamese ? skCrypt("Connecting to game...") : skCrypt("Connecting to game..."), 30, 7);
  std::cout << "\n";
  SetConsoleColor(14);
  int sync_count = 0;
  ULONGLONG engine_wait_started = GetTickCount64();
  ULONGLONG engine_connect_started = engine_wait_started;
  ULONGLONG last_engine_repick_tick = engine_wait_started;
  constexpr ULONGLONG kEngineRepickMs = 10000;
  constexpr ULONGLONG kEngineWaitDiagnosticMs = 30000;
  constexpr ULONGLONG kEngineInitHardFailMs = 90000;
  while (!AppShutdown::IsRequested() && !telemetryContext::Initialize(pid, base)) {
      sync_count++;
      const std::string initStatus = telemetryContext::GetLastInitializeStatus();
      
      // If we are stuck in decrypt-init-failed, show it clearly
      if (initStatus == skCrypt("decrypt-init-failed")) {
          std::cout << (g_is_vietnamese ? skCrypt("\r[*] Vui long vao game truoc... [") : skCrypt("\r[*] Please enter the game first... [")) << sync_count << skCrypt("]   ") << std::flush;
          UTN_DEV_LOG(std::cout << skCrypt(" status=decrypt-init-failed") << std::flush);
          Sleep(1500); // Wait a bit longer for game to settle
      } else {
          std::cout << (g_is_vietnamese ? skCrypt("\r[*] Hay vao game de tiep tuc... [") : skCrypt("\r[*] Enter the game to continue... [")) << sync_count << skCrypt("]...   ") << std::flush;
          UTN_DEV_LOG(std::cout << skCrypt(" status=") << initStatus << std::flush);
          Sleep(800);
      }

      if (sync_count % 5 == 0) {
          // Automatic recovery: Force clear cache and re-query process data
          telemetryMemory::HardResetProcessContext();
          
          query_process_data_packet live_data = {};
          const bool live_ok = telemetryHyperProcess::QueryProcessData(pid, &live_data);
          if (live_ok && live_data.process_id != 0) {
              const uint64_t live_base = reinterpret_cast<uint64_t>(live_data.base_address);
              telemetryMemory::g_ProcessCr3 = live_data.cr3;
              telemetryMemory::g_BaseAddress = live_base;
              base = live_base;
              UTN_DEV_LOG(std::cout << skCrypt(" [RESET-OK]") << std::flush);
          }
      }

      const ULONGLONG now_tick = GetTickCount64();
      if (now_tick - last_engine_repick_tick >= kEngineRepickMs) {
          last_engine_repick_tick = now_tick;

          ProcessUsageInfo repick = {};
          if (SelectBestTslGameProcess(&repick) && repick.pid != 0) {
              const uint64_t repick_base = reinterpret_cast<uint64_t>(repick.data.base_address);
              const bool changed_context =
                  repick.pid != pid ||
                  repick_base != telemetryMemory::g_BaseAddress ||
                  repick.data.cr3 != telemetryMemory::g_ProcessCr3;

              if (changed_context && telemetryMemory::AttachToGameStealthily(repick.pid)) {
                  pid = repick.pid;
                  base = telemetryMemory::g_BaseAddress;
                  telemetryDecrypt::Cleanup();
                  telemetryRuntimeOffsets::InvalidateRuntimeScanCache(base);
                  telemetryMemory::g_LastRefreshTime = 0;
                  engine_connect_started = GetTickCount64();
                  sync_count = 0;
                  std::cout << (g_is_vietnamese
                      ? skCrypt("\r[*] Dang ket noi lai game...      ")
                      : skCrypt("\r[*] Reconnecting to game...      ")) << std::flush;
                  UTN_DEV_LOG(std::cout << skCrypt("\n[DEV][ENGINE] repicked pid=") << pid
                      << skCrypt(" base=0x") << std::hex << base
                      << skCrypt(" cr3=0x") << telemetryMemory::g_ProcessCr3 << std::dec
                      << std::endl);
              }
          }
      }

      if (initStatus == skCrypt("ready")) {
          break;
      }
      if ((initStatus == skCrypt("decrypt-init-failed") ||
           initStatus == skCrypt("process-context-refresh-failed")) &&
          GetTickCount64() - engine_connect_started > kEngineInitHardFailMs) {
          StartupLog("engine-init-timeout");
          SetConsoleColor(12);
          UTN_DEV_LOG(std::cout << skCrypt("\n[DEV][ENGINE] Init stuck at status=")
                    << initStatus
                    << skCrypt(" for more than 90s.") << std::endl);
          std::cout << (g_is_vietnamese
              ? skCrypt("\n[-] Game chua san sang. Hay thoat han game roi mo lai game, sau do chay lai tool.\n")
              : skCrypt("\n[-] Game is not ready. Fully restart the game, then run the tool again.\n"));
          SetConsoleColor(7);
          DebugPause();
          return 1;
      }
      /*
      if (GetTickCount64() - engine_wait_started > kEngineWaitDiagnosticMs) {
          std::cout << skCrypt("\n[DEBUG][ENGINE] still waiting after 30s; last_status=")
                    << telemetryContext::GetLastInitializeStatus()
                    << skCrypt(". Retrying runtime scan periodically; if this repeats, restart the game process.") << std::endl;
          engine_wait_started = GetTickCount64();
      }
      */
      Sleep(1000 + (rand() % 200));
  }

  if (AppShutdown::IsRequested()) {
      telemetryDecrypt::Cleanup();
      SelfDestruct();
      return 0;
  }

  std::cout << (g_is_vietnamese ? skCrypt("\n[+] Ket noi thanh cong, hay vao game de trai nghiem!") : skCrypt("\n[+] Connection ready!")) << std::endl;
  SetConsoleColor(7);
  
    MacroEngine::Initialize();

    // EptMouse::InstallMouseEptHook(); // [TEMPORARY OFF]

    // [STEALTH] Robust Visualization Bridge Attachment (Loop until ready)
    VisualizationBridgeHost visualization_bridge = {};
    bool menu_initialized = false;
    int init_retry_count = 0;
    
    std::cout << (g_is_vietnamese ? skCrypt("[*] Dang chuan bi hien thi...\n") : skCrypt("[*] Preparing display...\n"));
    UTN_DEV_LOG(std::cout << skCrypt("[DEV] Synchronizing visualization bridge.") << std::endl);

    while (!AppShutdown::IsRequested() && !menu_initialized && init_retry_count < 60) {
        visualization_bridge = ResolvePassiveVisualizationHost();
        
        if (visualization_bridge.hwnd) {
            if (g_Menu.Initialize(visualization_bridge)) {
                menu_initialized = true;
                break;
            } else {
                UTN_DEV_LOG(std::cout << skCrypt("[-][DEV] Menu initialization failed; retrying.") << std::endl);
            }
        }

        init_retry_count++;
        Sleep(1000);
    }

    if (AppShutdown::IsRequested()) {
        telemetryDecrypt::Cleanup();
        SelfDestruct();
        return 0;
    }

    if (!menu_initialized) {
        StartupLog("visualization-bridge-failed");
        SetConsoleColor(12);
        std::cout << (g_is_vietnamese
            ? skCrypt("[-] Khong the khoi tao hien thi. Vui long thu lai.\n")
            : skCrypt("[-] Display could not be initialized. Please try again.\n"));
        UTN_DEV_LOG(std::cout << skCrypt("[-][DEV] Visualization bridge could not be synchronized after 60 seconds.") << std::endl);
        SetConsoleColor(7);
#ifndef _DEBUG
        MessageBoxA(NULL,
            skCrypt("Display could not be initialized.\nKhong the khoi tao hien thi."),
            skCrypt("GZ-telemetry - DISPLAY ERROR"), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_TOPMOST);
#endif
        DebugPause();
        SelfDestruct();
        return 1;
    }
    StartupLog("visualization-bridge-ok");

    /* [DKOM] Temporarily disabled to prevent window focus issues
    DWORD our_pid = GetCurrentProcessId();
    std::cout << skCrypt("[DKOM] Our PID: ") << our_pid << std::endl;
    uint64_t our_eprocess = telemetryHyperProcess::GetEProcessAddress(our_pid);
    if (our_eprocess) {
        telemetryHyperProcess::UnlinkProcessDKOM(our_eprocess);
    }
    */
    // [ANTI-DUMP] Safe Erasing DOS headers (Now compatible with DirectX)
    protec::erase_pe_header();
    
    std::cout << (g_is_vietnamese ? skCrypt("\n[+] He thong da san sang!") : skCrypt("\n[+] System Ready!")) << std::endl;
    std::cout << (g_is_vietnamese ? skCrypt("[+] Bam [F5] de Dong/Mo Menu | Bam [F11] de Tat Tool") : skCrypt("[+] F5: Menu | F11: Clean Exit")) << std::endl;
    StartupLog("system-ready");
    // RELEASE MODE: Use Bilingual System Modal (Top 1) MessageBox
    MessageBoxA(NULL, 
        skCrypt("System Ready!\nHe thong da san sang!\n\nPress [F5] for Menu. Press [F11] to exit.\nBam [F5] de Dong/Mo Menu. Bam [F11] de Tat Tool."), 
        skCrypt("GZ-telemetry - System Ready"), MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST);

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)[](LPVOID) {
        while (!AppShutdown::IsRequested()) {
            GameData.Keyboard.UpdateKeys();
            MacroEngine::Update();
            Sleep(1);
        }
        return (DWORD)0;
    }, NULL, 0, NULL);

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)[](LPVOID) {
      while (!AppShutdown::IsRequested()) {
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
