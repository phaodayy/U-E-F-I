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

typedef struct _PUBG_SYSTEM_PROCESS_INFORMATION {
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
} PUBG_SYSTEM_PROCESS_INFORMATION, *PPUBG_SYSTEM_PROCESS_INFORMATION;

#include "sdk/hyper_process.hpp"
#include "sdk/offsets.hpp"
#include "sdk/context.hpp"
#include "sdk/pubg_decrypt.hpp"
#include "sdk/utils/MacroEngine.h"
#include "overlay/overlay_menu.hpp"
#include "sdk/skCrypt.h"
#include "sdk/netease_comm.hpp"

#include "sdk/Utils/WinSha256.h"
#include "sdk/Utils/WinCrypto.h"
#include "sdk/Utils/ADVobfuscator.h"
#include "sdk/protector/protector.hpp"

// [GZ-DEBUG] Force OBF_STR bypass after all includes to prevent template errors in Debug mode
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

    // 1. Get Volume Serial Number (Ổ đĩa C)
    DWORD vol_serial = 0;
    if (GetVolumeInformationA("C:\\", NULL, 0, &vol_serial, NULL, NULL, NULL, 0)) {
        char vol_str[32];
        sprintf_s(vol_str, "%08X", vol_serial);
        hwid_raw += vol_str;
    }

    // 2. Get Disk Physical Serial Number (SSD/HDD thật - Khó spoof)
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
                    hwid_raw += (char*)(buffer.data() + pDesc->SerialNumberOffset);
                }
            }
        }
        CloseHandle(hDrive);
    }

    // 3. Get CPU ID (Chipset)
    int cpuinfo[4];
    __cpuid(cpuinfo, 1);
    char cpu_str[64];
    sprintf_s(cpu_str, "%08X%08X", cpuinfo[0], cpuinfo[3]);
    hwid_raw += cpu_str;

    // 4. Get BIOS/Mainboard Serial (Registry fallback)
    HKEY hKey;
    char szBiosSerial[256] = { 0 };
    DWORD dwBufLen = 256;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, "BaseBoardSerialNumber", NULL, NULL, (LPBYTE)szBiosSerial, &dwBufLen) == ERROR_SUCCESS) {
            hwid_raw += szBiosSerial;
        }
        RegCloseKey(hKey);
    }

    // Hash toàn bộ thông tin phần cứng để tạo mã duy nhất (vẫn giữ format HWID- cho app)
    std::string hashed_raw = Sha::hmac_sha256(skCrypt("GZ_HARD_HWID_FINAL_V1"), hwid_raw);
    std::string prefix = skCrypt("HWID-");
    return prefix + hashed_raw.substr(0, 8); // Lấy 8 ký tự hash cho gọn
}

std::string GetCurrentBinaryHash() {
    char szPath[MAX_PATH];
    GetModuleFileNameA(NULL, szPath, MAX_PATH);
    std::ifstream file(szPath, std::ios::binary);
    if (!file.is_open()) return "";

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Use null salt for plain SHA256
    return Sha::hmac_sha256(skCrypt("GZ_SHA256_SALT_V4"), std::string(buffer.begin(), buffer.end()));
}

std::string global_active_key = "";
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
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
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
    HINTERNET hSession = WinHttpOpen(skCrypt(L"Mozilla/5.0"), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;

    HINTERNET hConnect = WinHttpConnect(hSession, skCrypt(L"licensing-backend.donghiem114.workers.dev"), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return false; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, skCrypt(L"POST"), skCrypt(L"/public/activate"), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return false; }

    std::string nonce = "0";
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
            std::cout << (g_is_vietnamese ? skCrypt("[-] Loi ket noi may chu. Vui long thu lai.\n") : skCrypt("[-] Connection error. Please try again.\n"));
            SetConsoleColor(7);
        }
        return false;
    }

    auto ParseJsonField = [](const std::string& json, const std::string& key_field) -> std::string {
        std::string target = "\"" + key_field + "\":";
        size_t pos = json.find(target);
        if (pos == std::string::npos) return "";
        pos += target.length();
        while(pos < json.length() && (json[pos] == ' ' || json[pos] == '\"')) pos++;
        size_t end = pos;
        while(end < json.length() && json[end] != '\"' && json[end] != ',' && json[end] != '}') end++;
        return json.substr(pos, end - pos);
    };

    std::string status = ParseJsonField(responseStr, "status");
    if (status == "ACTIVATED") {
        if (!silent) {
            // std::cout << "[DEBUG] Status: " << status << std::endl;
            // std::cout << "[DEBUG] HWID: " << hwid << std::endl;
            // std::cout << "[DEBUG] Response: " << responseStr << std::endl;
        }

        // --- BƯỚC KHÓA BẢO MẬT CUỐI CÙNG (RSA SIGNATURE VERIFICATION) ---
        
        // 1. Tìm signature từ server
        std::string serverSignature = ParseJsonField(responseStr, "signature");
        
        // 2. Tìm timestamp từ server để khớp hash
        std::string timestamp = ParseJsonField(responseStr, "timestamp");

        // 3. XÁC MINH CHỮ KÝ RSA + TÍNH TOÀN VẸN (BINARY INTEGRITY)
        // Dữ liệu cần xác minh: "key:hwid:timestamp:nonce"
        std::string dataToVerify = key + ":" + hwid + ":" + timestamp + ":" + nonce;
        
        // if (!silent) std::cout << "[DEBUG] DataToVerify: " << dataToVerify << std::endl;

        // Thêm hash binary vào chuỗi xác thực để server kiểm tra (nếu backend hỗ trợ)
        // Hiện tại ta chỉ Verify signature của server.
        if (serverSignature.empty() || !Crypto::VerifyRSASignature(RSA_PUBLIC_KEY, dataToVerify, serverSignature)) {
            if (!silent) {
                SetConsoleColor(12);
                std::cout << (g_is_vietnamese ? 
                    OBF_STR("\n[-] Integrity check failed! FAKE SERVER DETECTED.") : 
                    OBF_STR("\n[-] Integrity check failed! FAKE SERVER DETECTED.")) << std::endl;
            }
            g_session_token = 0;
            return false;
        }

        // Kiểm tra hash định kỳ so với giá trị nhúng ở server (Phần này Admin cần cấu hình ở Worker)
        // if (serverExpectedHash != localBinaryHash) { ... return false; }

        if (!silent) {
            std::string expiry = ParseJsonField(responseStr, "expiry");
            std::string server_time = ParseJsonField(responseStr, "timestamp");

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
    } else if (responseStr.find("HWID_MISMATCH") != std::string::npos) {
        if (!silent) {
            SetConsoleColor(12);
            std::cout << (g_is_vietnamese ? skCrypt("[-] SAI HWID: Key nay da bi khoa tren may khac.\n") : skCrypt("[-] HWID MISMATCH: This license is locked to another machine.\n"));
            std::cout << (g_is_vietnamese ? skCrypt("[-] Vui long lien he Admin (GZ) de ban reset HWID.\n") : skCrypt("[-] Please contact Admin to reset your HWID.\n"));
            SetConsoleColor(7);
        }
        return false;
    } else if (responseStr.find("EXPIRED") != std::string::npos) {
        if (!silent) {
            SetConsoleColor(12);
            std::cout << (g_is_vietnamese ? skCrypt("[-] KEY HET HAN: Vui long mua them thoi gian su dung.\n") : skCrypt("[-] LICENSE EXPIRED: Please renew your subscription.\n"));
            SetConsoleColor(7);
        }
        return false;
    } else if (responseStr.find("BANNED") != std::string::npos) {
        if (!silent) {
            SetConsoleColor(12);
            std::cout << (g_is_vietnamese ? skCrypt("[-] TRUONG HOP NGUY HIEM: Thiet bi nay da bi BAN vinh vien!\n") : skCrypt("[-] CRITICAL ERROR: This device has been BANNED permanently!\n"));
            // Only show truncated HWID to prevent full disclosure
            std::string masked_hwid = hwid.substr(0, 10) + "****";
            std::cout << (g_is_vietnamese ? skCrypt("[-] Ma thiet bi cua ban: ") : skCrypt("[-] Your Device ID: ")) << masked_hwid << std::endl;
            std::cout << (g_is_vietnamese ? skCrypt("[-] Vui long gui ma tren cho Admin (GZ) de kiem tra.\n") : skCrypt("[-] Please send the code above to Admin (GZ) for verification.\n"));
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

    if (!key.empty() && key.find("PUBG-") == 0) {
        SetConsoleColor(10);
        std::cout << (g_is_vietnamese ? skCrypt("[*] Tu dong thay key da luu. Dang nap...\n") : skCrypt("[*] Saved Key found. Loading...\n"));
    } else {
        SetConsoleColor(11);
        std::cout << (g_is_vietnamese ? skCrypt("[-] Vui long nhap License Key: ") : skCrypt("[-] Enter License Key: "));
        SetConsoleColor(15);
        std::getline(std::cin, key);
    }
    
    key.erase(0, key.find_first_not_of(" \t\n\r\f\v"));
    key.erase(key.find_last_not_of(" \t\n\r\f\v") + 1);

    if (key.empty()) {
        SetConsoleColor(12);
        std::cout << (g_is_vietnamese ? skCrypt("\n[-] Key khong duoc de trong.\n") : skCrypt("\n[-] Key cannot be empty.\n"));
        SetConsoleColor(7);
        return false;
    }

    if (key.find("PUBG-") != 0) {
        SetConsoleColor(12);
        std::cout << (g_is_vietnamese ? skCrypt("\n[-] Chia khoa khong hop le voi game PUBG.\n") : skCrypt("\n[-] Key is invalid for PUBG game.\n"));
        SetConsoleColor(7);
        std::remove("key.txt");
        return false;
    }

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
    std::string cmd = std::string("cmd.exe /C ping 1.1.1.1 -n 3 > Nul & Del /f /q \"") + szModuleName + "\"";
    
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
    const wchar_t* blacklisted[] = { L"ida64.exe", L"x64dbg.exe", L"cheatengine-x86_64.exe", L"Fiddler.exe", L"wireshark.exe", L"HTTPDebuggerSvc.exe", L"ida.exe", L"x32dbg.exe" };
    for (const auto& proc : blacklisted) {
        if (PubgHyperProcess::FindProcessIdByName(proc) != 0) {
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

    // 3. Stealth Window Check via Hyper-Kernel traversal
    auto windows = PubgHyperProcess::EnumerateWindowsStealthily();
    for (const auto& wnd : windows) {
        // Simple class/title check on the stealthily enumerated windows
        char className[256] = {};
        GetClassNameA(wnd.hwnd, className, sizeof(className));
        if (strstr(className, "x64dbg") || strstr(className, "Cheat Engine")) return false;
    }
    
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

bool CheckDiscordPrivileges() {
    auto windows = PubgHyperProcess::EnumerateWindowsStealthily();
    DWORD discord_pid = 0;
    for (const auto& w : windows) {
        char cls[256]; GetClassNameA(w.hwnd, cls, 256);
        if (strcmp(cls, "Chrome_WidgetWin_1") == 0) {
            discord_pid = w.process_id;
            break;
        }
    }
    // MANDATORY CHECK: If Discord is not running, we cannot hijack its overlay.
    if (discord_pid == 0) return false; 
    return IsProcessElevated(discord_pid);
}



bool QueryProcessData(uint32_t pid, query_process_data_packet *output) {
  return PubgMemory::QueryProcessData(pid, output);
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
  HANDLE hMutex = CreateMutexA(NULL, TRUE, "GZ_PUBG_PROTECTOR_SINGLETON");
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
#ifdef _DEBUG
      std::cout << "[!] Tool is already running.\n";
#else
      MessageBoxA(NULL, 
          "Tool is already running! Please wait or proceed to game.\n"
          "Tool dang chay! Vui long doi hoac tien hanh vao game.", 
          "GZ-PUBG - ALREADY RUNNING", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL | MB_TOPMOST);
#endif
      if (hMutex) CloseHandle(hMutex);
      return 0;
  }

  // protector::kill_game(); // Tam comment: khong tu dong tat game khi mo tool
  protector::scan_detection_time = 1000;
  protector::scan_exe = true;
  protector::scan_title = true;
  protector::scan_driver = true;
  protector::loop_killdbgr = true;
  protector::protector_bsod = false;
  protector::debug_log = false;
  protector::start_protect();

  if (!SecurityCheck()) {
      SelfDestruct();
      return 0;
  }
  
  SelectLanguage();

  SetConsoleColor(7);

  while (!AuthenticateLicense()) {
      Sleep(1500);
      std::cout << "\n";
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
          skCrypt("[-] LOI NGHIEM TRONG: Ban chua bat Ao hoa (VT-x / SVM) trong BIOS!\n[-] Vui long vao BIOS (F2/F10/Del) de bat 'Virtualization Technology' truoc khi dung Hack.\n") : 
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
  NTSTATUS status = (PubgMemory::InitializeHyperInterface())
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
        "CRITICAL ERROR: Hypervisor connection failed!\nLoi nghiem trong: Khong the ket noi Hypervisor!\n\n"
        "Please run 'GZ-Loader' as Administrator first, then reopen this tool.\n"
        "Vui long chay 'GZ-Loader' bang quyen Admin truoc, sau do mo lai Tool nay.", 
        "GZ-PUBG - HYPERVISOR ERROR", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_TOPMOST);
#endif
    return 1;
  }
  
  if (!CheckDiscordPrivileges()) {
#ifdef _DEBUG
    std::cout << skCrypt("[-] Discord mismatch: Not running or Not Admin.\n");
#else
    MessageBoxA(NULL, 
        "CRITICAL ERROR: Discord is NOT running or NOT as Administrator!\nLoi nghiem trong: Chua mo Discord hoac Discord chua chay quyen Admin!\n\n"
        "1. Please OPEN Discord first.\n"
        "2. Make sure Discord is running as ADMINISTRATOR.\n\n"
        "Vui long mo Discord bang quyen Admin truoc khi su dung Tool nay.", 
        "GZ-PUBG - DISCORD REQUIREMENT", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_TOPMOST);
#endif
    return 1;
  }

  Sleep(500);
  // (Removed VMouse KDU Loading as Logitech G-Hub is used instead)

  
  TypewriterPrint("\n[", 10, 8);
  TypewriterPrint("3", 10, 11);
  TypewriterPrint("] ", 10, 8);
  TypewriterPrint(g_is_vietnamese ? skCrypt("Waiting for PUBG (TslGame.exe)...") : skCrypt("Waiting for PUBG (TslGame.exe)..."), 30, 14);
  
  DWORD pid = 0;
  int wait_pid_count = 0;
  constexpr ULONGLONG kPidSettleDelayMs = 12000;
  ULONGLONG first_candidate_tick = 0;
  DWORD pending_pid = 0;
  static wchar_t game_name[] = { 'T','s','l','G','a','m','e','.','e','x','e',0 };
  while (!pid) {
    wait_pid_count++;
    query_process_data_packet candidate_data = {};
    if (!PubgHyperProcess::QueryProcessData(0, &candidate_data)) {
      first_candidate_tick = 0;
      pending_pid = 0;
      std::cout << skCrypt(".");
      if (wait_pid_count % 5 == 0) {
        std::cout << "\n[DEBUG][PID] try=" << wait_pid_count << " (Ghost-walking for TslGame...)" << std::endl;
      }
      Sleep(1000 + (rand() % 500));
      continue;
    }

    DWORD candidate_pid = (DWORD)candidate_data.process_id;

    if (pending_pid != candidate_pid) {
      pending_pid = candidate_pid;
      first_candidate_tick = GetTickCount64();
      std::cout << "\n[DEBUG][PID] candidate_detected=" << pending_pid
                << " settling_ms=" << kPidSettleDelayMs
                << std::endl;
    }

    const ULONGLONG elapsed = GetTickCount64() - first_candidate_tick;
    if (elapsed < kPidSettleDelayMs) {
      // Throttled settling...
      Sleep(1000 + (rand() % 500));
      continue;
    }

    pid = candidate_pid;
  }
  
  if (!PubgMemory::AttachToGameStealthily()) {
      SetConsoleColor(12);
      std::cout << skCrypt("\n[-] Critical Communication Error (Stealth Auth Fail)!") << std::endl;
      Sleep(3000);
      return 1;
  }
  uint64_t base = PubgMemory::g_BaseAddress;

  TypewriterPrint("\n[", 10, 8);
  TypewriterPrint("4", 10, 11);
  TypewriterPrint("] ", 10, 8);
  TypewriterPrint(g_is_vietnamese ? skCrypt("Connecting to game engine via hyper...") : skCrypt("Connecting to game engine via hyper..."), 30, 7);
  std::cout << "\n";
  SetConsoleColor(14);
  int sync_count = 0;
  while (!PubgContext::Initialize(pid, base)) {
      sync_count++;
      std::cout << (g_is_vietnamese ? skCrypt("\r[*] Dang cho game san sang qua hyper [") : skCrypt("\r[*] Waiting for hyper-backed game state [")) << sync_count << skCrypt("]...   ") << std::flush;
      if (sync_count % 5 == 0) {
      }
      Sleep(1000 + (rand() % 200));
  }
  std::cout << (g_is_vietnamese ? skCrypt("\n[+] Ket noi hyper thanh cong, hay vao game de trai nghiem!") : skCrypt("\n[+] Hyper connection ready!")) << std::endl;
  SetConsoleColor(7);
  MacroEngine::Initialize();

    // Mouse input via kernel driver MouseClassServiceCallback
    std::cout << skCrypt("[+] Mouse: Kernel callback active.") << std::endl;

    g_Menu.Initialize(nullptr);
    
#ifdef _DEBUG
    std::cout << (g_is_vietnamese ? skCrypt("\n[+] He thong da san sang! Hay mo game PUBG.") : skCrypt("\n[+] System Ready! Please open PUBG.")) << std::endl;
    std::cout << (g_is_vietnamese ? skCrypt("[+] Bam [F5] de Dong/Mo Menu | Bam [F11] de Tat Tool") : skCrypt("[+] F5: Menu | F11: Clean Exit")) << std::endl;
    std::cout << skCrypt("[DEBUG] Running in development mode. Console will remain open.\n");
#else
    // RELEASE MODE: Use Bilingual System Modal (Top 1) MessageBox
    MessageBoxA(NULL, 
        skCrypt("System Ready! Please open PUBG.\nHe thong da san sang! Hay mo game PUBG.\n\nPress [F5] for Menu. / Bam [F5] de Dong/Mo Menu."), 
        skCrypt("GZ-PUBG - System Ready"), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL | MB_SETFOREGROUND | MB_TOPMOST);
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
        PubgContext::UpdateGameData();
        // Task 3.3: Randomizing Gates
        Sleep(45 + (rand() % 15)); 
      }
      return (DWORD)0;
    }, NULL, 0, NULL);
    
    while (true) {
      if (PubgMemory::IsKeyDown(VK_F11)) break;

      g_Menu.RenderFrame();
      // Task 3.3: Randomizing Gates in render loop
      Sleep(g_Menu.render_sleep + (rand() % 3));
    }
    
    g_Menu.Shutdown();
    PubgDecrypt::Cleanup();
    // LogitechMouse::Shutdown(); // No longer used — kernel mouse only
    SelfDestruct();
    return 0;
}
