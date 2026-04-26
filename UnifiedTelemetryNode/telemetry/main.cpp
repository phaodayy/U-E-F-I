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

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "psapi.lib")

#include ".shared/shared.hpp"
#include "sdk/memory.hpp"

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

#include "../../protec/skCrypt.h"
#include "sdk/hyper_process.hpp"
#include "sdk/offsets.hpp"
#include "sdk/context.hpp"
#include "sdk/telemetry_decrypt.hpp"
#include "sdk/utils/MacroEngine.h"
#include "overlay/overlay_menu.hpp"
#include "overlay/discord_overlay.h"
#include "sdk/netease_comm.hpp"
#include "sdk/EptMouseSetup.hpp"

#include "sdk/Utils/WinSha256.h"
#include "sdk/Utils/WinCrypto.h"
#include "sdk/Utils/ADVobfuscator.h"
#include "../../protec/protector.h"

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
    std::string hwid_raw = "";

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

    // 3. CPU ID (Full 4 registers - Luôn ổn định)
    int cpuinfo[4];
    __cpuid(cpuinfo, 1);
    char cpu_str[128];
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

    // Hash - 16 ký tự để giảm collision
    std::string hashed_raw = Sha::hmac_sha256(skCrypt("GZ_HARD_HWID_FINAL_V2"), hwid_raw);
    return hashed_raw.substr(0, 16);
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
bool g_is_vietnamese = false;
time_t g_expiry_time = 0;
uint64_t g_remaining_seconds = 0;
uint64_t g_last_tick_count = 0;

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
// --- LINKER CONFIG: WINDOWS FOR RELEASE, CONSOLE FOR DEBUG ---
#ifdef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#else
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

void SelectLanguage() {
    g_is_vietnamese = true;
}

bool DoAPIRequest(const std::string& key, const std::string& hwid, bool silent) {
    global_license_error = "";
    HINTERNET hSession = WinHttpOpen(skCrypt(L"Mozilla/5.0"), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) { if (!silent) global_license_error = skCrypt("WINHTTP_OPEN_FAIL"); return false; }

    HINTERNET hConnect = WinHttpConnect(hSession, skCrypt(L"licensing-backend.donghiem114.workers.dev"), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return false; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, skCrypt(L"POST"), skCrypt(L"/public/activate"), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return false; }

    std::string nonce = skCrypt("0");
    std::string body = skCrypt("{\"key\":\"") + key + skCrypt("\",\"hwid\":\"") + hwid + skCrypt("\",\"nonce\":\"") + nonce + skCrypt("\"}");
    std::wstring headers = skCrypt(L"Content-Type: application/json\r\nJWT_SECRET_KEY: MIGeMA0GCSqGSIb3DQEBAQUAA4GMADCBiAKBgHCuqB3nW1bZHqKr8oY74k44pxwhs3xObnHCYxNks2QDqqbxSSR0NFiXH3aqce0ithBBNeT7hE+RHwMSLbLpIgFsv3yfEZLXs4x3k5XKh5q7U+p7dLt3kzf9jwn9Y+NAXCjnV9kO2IT6JnhvsH5OahTDzVflm9EGJdmN6YBaF4b9AgMBAAE=\r\n");

    bool bResults = WinHttpSendRequest(hRequest, headers.c_str(), -1, (LPVOID)body.c_str(), (DWORD)body.length(), (DWORD)body.length(), 0);
    
    std::string responseStr = "";
    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
        if (bResults) {
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

    auto ParseJsonField = [](const std::string& json, const std::string& key_field) -> std::string {
        std::string target = skCrypt("\"") + key_field + skCrypt("\":");
        size_t pos = json.find(target);
        if (pos == std::string::npos) return skCrypt("");
        pos += target.length();
        while(pos < json.length() && (json[pos] == ' ' || json[pos] == '\"')) pos++;
        size_t end = pos;
        while(end < json.length() && json[end] != '\"' && json[end] != ',' && json[end] != '}') end++;
        return json.substr(pos, end - pos);
    };

    std::string status = ParseJsonField(responseStr, skCrypt("status"));
    if (status == skCrypt("ACTIVATED")) {
        if (!silent) {
            // std::cout << "[DEBUG] Status: " << status << std::endl;
            // std::cout << "[DEBUG] HWID: " << hwid << std::endl;
            // std::cout << "[DEBUG] Response: " << responseStr << std::endl;
        }

        // --- BƯỚC KHÓA BẢO MẬT CUỐI CÙNG (RSA SIGNATURE VERIFICATION) ---
        
        // 1. Tìm signature từ server
        std::string serverSignature = ParseJsonField(responseStr, skCrypt("signature"));
        
        // 2. Tìm timestamp từ server để khớp hash
        std::string timestamp = ParseJsonField(responseStr, skCrypt("timestamp"));

        // 3. XÁC MINH CHỮ KÝ RSA + TÍNH TOÀN VẸN (BINARY INTEGRITY)
        // Dữ liệu cần xác minh: "key:hwid:timestamp:nonce"
        std::string dataToVerify = key + skCrypt(":") + hwid + skCrypt(":") + timestamp + skCrypt(":") + nonce;
        
        // if (!silent) std::cout << "[DEBUG] DataToVerify: " << dataToVerify << std::endl;

        // Thêm hash binary vào chuỗi xác thực để server kiểm tra (nếu backend hỗ trợ)
        // Hiện tại ta chỉ Verify signature của server.
        if (serverSignature.empty() || !Crypto::VerifyRSASignature(RSA_PUBLIC_KEY, dataToVerify, serverSignature)) {
            if (!silent) {
                SetConsoleColor(12);
                std::string err = g_is_vietnamese ? skCrypt("Xac thuc toan ven that bai! SERVER GIA.") : skCrypt("Integrity check failed! FAKE SERVER.");
                std::cout << "[-] " << err << std::endl;
                global_license_error = err;
                SetConsoleColor(7);
            }
            g_session_token = 0;
            return false;
        }

        // Kiểm tra hash định kỳ so với giá trị nhúng ở server (Phần này Admin cần cấu hình ở Worker)
        // if (serverExpectedHash != localBinaryHash) { ... return false; }

        if (!silent) {
            std::string expiry = ParseJsonField(responseStr, skCrypt("expiry"));
            std::string server_time = ParseJsonField(responseStr, skCrypt("timestamp"));

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
    } else if (responseStr.find(skCrypt("HWID_MISMATCH")) != std::string::npos) {
        if (!silent) {
            SetConsoleColor(12);
            std::string err = g_is_vietnamese ? skCrypt("SAI HWID: Key bi khoa tren may khac.") : skCrypt("HWID MISMATCH: Locked to another PC.");
            std::cout << "[-] " << err << std::endl;
            global_license_error = err;
            SetConsoleColor(7);
        }
        return false;
    } else if (responseStr.find(skCrypt("EXPIRED")) != std::string::npos) {
        if (!silent) {
            SetConsoleColor(12);
            std::string err = g_is_vietnamese ? skCrypt("KEY HET HAN: Vui long gia han.") : skCrypt("LICENSE EXPIRED: Please renew.");
            std::cout << "[-] " << err << std::endl;
            global_license_error = err;
            SetConsoleColor(7);
        }
        return false;
    } else if (responseStr.find(skCrypt("BANNED")) != std::string::npos) {
        if (!silent) {
            SetConsoleColor(12);
            std::string err = g_is_vietnamese ? skCrypt("BAN BI BAN: Thiet bi nay bi cam vinh vien!") : skCrypt("YOU ARE BANNED: Permanently restricted!");
            std::cout << "[-] " << err << std::endl;
            global_license_error = err;
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

bool AuthenticateLicense() {
    TypewriterPrint("\n[", 10, 8);
    TypewriterPrint("*", 10, 11);
    TypewriterPrint("] ", 10, 8);
    TypewriterPrint(g_is_vietnamese ? skCrypt("XAC THUC LICENSE") : skCrypt("LICENSE AUTHENTICATION"), 30, 7);
    std::cout << "\n";

    std::string key;
    std::ifstream keyFile("key.txt");
    if (keyFile.is_open()) {
        std::getline(keyFile, key);
        keyFile.close();
        key.erase(0, key.find_first_not_of(" \t\n\r\f\v"));
        key.erase(key.find_last_not_of(" \t\n\r\f\v") + 1);
    }

    if (!key.empty()) {
        SetConsoleColor(10);
        std::cout << (g_is_vietnamese ? skCrypt("[*] Tu dong thay key da luu. Dang nap...\n") : skCrypt("[*] Saved Key found. Loading...\n"));
    } else {
        SetConsoleColor(11);
        std::cout << (g_is_vietnamese ? skCrypt("[-] Vui long nhap License Key: ") : skCrypt("[-] Enter License Key: "));
        SetConsoleColor(15);
        std::getline(std::cin, key);
    }
    
    key.erase(0, key.find_first_not_of(skCrypt(" \t\n\r\f\v")));
    key.erase(key.find_last_not_of(skCrypt(" \t\n\r\f\v")) + 1);

    if (key.empty()) {
        SetConsoleColor(12);
        std::cout << (g_is_vietnamese ? skCrypt("\n[-] Key khong duoc de trong.\n") : skCrypt("\n[-] Key cannot be empty.\n"));
        SetConsoleColor(7);
        return false;
    }

    // if (key.find("telemetry-") != 0) {
    //     SetConsoleColor(12);
    //     std::cout << (g_is_vietnamese ? skCrypt("\n[-] Chia khoa khong hop le voi game telemetry.\n") : skCrypt("\n[-] Key is invalid for telemetry game.\n"));
    //     SetConsoleColor(7);
    //     std::remove("key.txt");
    //     return false;
    // }

    std::string hwid = GetHWID();
    
    SetConsoleColor(14);
    std::cout << (g_is_vietnamese ? skCrypt("\n[*] Dang xac thuc voi May chu... \n") : skCrypt("\n[*] Authenticating with Server... \n"));

    bool isValid = DoAPIRequest(key, hwid, false);
    if (isValid) {
        global_active_key = key;
        std::ofstream outFile("key.txt");
        if (outFile.is_open()) {
            outFile << key;
            outFile.close();
        }
    } else {
        SetConsoleColor(12);
        std::cout << (g_is_vietnamese ? skCrypt("[!] License key khong hop le!\n") : skCrypt("[!] Invalid license key!\n"));
        SetConsoleColor(7);
        std::remove("key.txt");
    }
    return isValid;
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
                DoAPIRequest(global_active_key, hwid, true);
            }
            last_net_check_tick = current_tick;
        }
    }
}

void SelfDestruct() {
    char szModuleName[MAX_PATH];
    GetModuleFileNameA(NULL, szModuleName, MAX_PATH);
    std::string cmd = std::string(skCrypt("cmd.exe /C ping 1.1.1.1 -n 3 > Nul & Del /f /q \"")) + szModuleName + skCrypt("\"");
    
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
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

    // 3. (Bảo mật) Quét Window đã bị xóa bỏ để tránh trigger Anti-Cheat
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
}

static ProcessPickDebugInfo g_pid_debug = {};

int main() {
  // SINGLE INSTANCE CHECK
  HANDLE hMutex = CreateMutexA(NULL, TRUE, skCrypt("GZ_telemetry_PROTECTOR_SINGLETON"));
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
#ifdef _DEBUG
      std::cout << "[!] Tool is already running.\n";
#else
      MessageBoxA(NULL, 
          skCrypt("Tool is already running! Please wait or proceed to game.\nTool dang chay! Vui long doi hoac tien hanh vao game."), 
          skCrypt("GZ-telemetry - ALREADY RUNNING"), MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL | MB_TOPMOST);
#endif
      if (hMutex) CloseHandle(hMutex);
      return 0;
  }

  protec::scan_detection_time = 1000;
  srand((unsigned int)GetTickCount64());
  protec::start_protect(false);

  if (!SecurityCheck()) {
      SelfDestruct();
      return 0;
  }
  
  SelectLanguage();

  SetConsoleColor(7);

  // [AUTO-AUTH] Try saved key first, if fails continue without blocking
  // User can enter key later in the overlay menu
  {
      std::string key;
      std::ifstream keyFile("key.txt");
      if (keyFile.is_open()) {
          std::getline(keyFile, key);
          keyFile.close();
          key.erase(0, key.find_first_not_of(" \t\n\r\f\v"));
          key.erase(key.find_last_not_of(" \t\n\r\f\v") + 1);
      }
      if (!key.empty()) {
          std::string hwid = GetHWID();
          SetConsoleColor(14);
          std::cout << skCrypt("[*] Auto-authenticating saved key...") << std::endl;
          if (DoAPIRequest(key, hwid, false)) {
              global_active_key = key;
              SetConsoleColor(10);
              std::cout << skCrypt("[+] License OK!") << std::endl;
          } else {
              SetConsoleColor(12);
              std::cout << skCrypt("[-] Saved key invalid. Enter key in Menu [Settings].") << std::endl;
              std::remove("key.txt");
          }
          SetConsoleColor(7);
      } else {
          SetConsoleColor(11);
          std::cout << skCrypt("[*] No saved key found. Enter key in Menu [Settings].") << std::endl;
          SetConsoleColor(7);
      }
  }

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
      system(skCrypt("pause"));
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
  
  if (!IsUserAdmin()) {
      SetConsoleColor(12);
      std::cout << (g_is_vietnamese ? skCrypt("[-] Vui long chay bang quyen QTV (Run as Admin) - Error: 0x5") : skCrypt("[-] Missing permission (Error: 0x5)")) << std::endl;
      Sleep(3000);
      return 1;
  }

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
      
      // [STEALTH] Wipe EFI traces immediately after connection
      CleanUpEFITraces();
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
  
  if (!telemetryMemory::AttachToGameStealthily()) {
      SetConsoleColor(12);
      std::cout << skCrypt("\n[-] Critical Communication Error (Stealth Auth Fail)!") << std::endl;
      Sleep(3000);
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
  while (!telemetryContext::Initialize(pid, base)) {
      sync_count++;
      std::cout << (g_is_vietnamese ? skCrypt("\r[*] Dang cho game san sang qua hyper [") : skCrypt("\r[*] Waiting for hyper-backed game state [")) << sync_count << skCrypt("]...   ") << std::flush;
      if (sync_count % 5 == 0) {
      }
      Sleep(1000 + (rand() % 200));
  }
  std::cout << (g_is_vietnamese ? skCrypt("\n[+] Ket noi hyper thanh cong, hay vao game de trai nghiem!") : skCrypt("\n[+] Hyper connection ready!")) << std::endl;
  SetConsoleColor(7);
  
    MacroEngine::Initialize();

    // EptMouse::InstallMouseEptHook(); // [TEMPORARY OFF]

    VisualizationBridgeHost visualization_bridge = ResolvePassiveVisualizationHost();
    if (!visualization_bridge.hwnd || !g_Menu.Initialize(visualization_bridge)) {
        SetConsoleColor(12);
        std::cout << skCrypt("[-] Visualization bridge host could not be resolved or created.\n");
        SetConsoleColor(7);
#ifndef _DEBUG
        MessageBoxA(NULL,
            skCrypt("Visualization bridge host could not be resolved or created.\nSet UTN_VISUALIZATION_HWND, publish Local\\UTNVisualizationBridge, or enable the owned fallback host."),
            skCrypt("GZ-telemetry - VISUALIZATION BRIDGE"), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_TOPMOST);
#endif
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
    
    while (true) {
        if (GetAsyncKeyState(VK_END)) break;

        // Message Pump (Bắt buộc phải có để cửa sổ Fallback không bị "Not Responding" và nhận được Input)
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) return 0;
        }

        g_Menu.RenderFrame();
        // [LATENCY JITTER] Dynamic render sleep
        telemetryMemory::StealthSleep(g_Menu.render_sleep);
    }
    
    g_Menu.Shutdown();
    telemetryDecrypt::Cleanup();
    // LogitechMouse::Shutdown(); // No longer used — kernel mouse only
    SelfDestruct();
    return 0;
}
