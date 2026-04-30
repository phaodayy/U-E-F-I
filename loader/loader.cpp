#include <windows.h>
#include <string>
#include <shlobj.h>
#include <iomanip>
#include <fstream>
#include <vector>
#include <sstream>
#include <winhttp.h>
#include <wincrypt.h>
#include <winioctl.h>
#include <ntddstor.h>
#include <intrin.h>
#include "../UnifiedTelemetryNode/nlohmann/json.hpp"
#include "../UnifiedTelemetryNode/telemetry/sdk/Utils/WinSha256.h"
#include "resource.h"
#include "../protec/protector.h"

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Winhttp.lib")
#pragma comment(lib, "Bcrypt.lib")

// --- LINKER CONFIG: WINDOWS FOR RELEASE, CONSOLE FOR DEBUG ---
#ifdef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#else
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

void print_log(const std::string& msg) {
#ifdef _DEBUG
    std::string line = msg + "\n";
    OutputDebugStringA(line.c_str());
#else
    (void)msg;
#endif
}

bool extract_resource(int resId, const std::string& outPath) {
    HRSRC hRes = FindResourceA(NULL, MAKEINTRESOURCEA(resId), RT_RCDATA);
    if (!hRes) return false;

    HGLOBAL hGlobal = LoadResource(NULL, hRes);
    if (!hGlobal) return false;

    void* pData = LockResource(hGlobal);
    DWORD size = SizeofResource(NULL, hRes);

    std::ofstream outFile(outPath, std::ios::binary);
    if (!outFile.is_open()) return false;

    outFile.write((const char*)pData, size);
    outFile.close();
    return true;
}

bool is_admin() {
    BOOL admin = FALSE;
    PSID admin_group;
    SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &admin_group)) {
        CheckTokenMembership(NULL, admin_group, &admin);
        FreeSid(admin_group);
    }
    return admin == TRUE;
}

void run_as_admin() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    ShellExecuteA(NULL, "runas", path, NULL, NULL, SW_SHOWNORMAL);
}

bool aggressive_copy(const std::string& src, const std::string& dst) {
    system(("attrib -s -h -r \"" + dst + "\" >nul 2>&1").c_str());
    DeleteFileA(dst.c_str());
    if (CopyFileA(src.c_str(), dst.c_str(), FALSE)) {
#ifdef _DEBUG
        print_log("[+] SUCCESS: Updated " + dst);
#endif
        return true;
    }
    print_log("[-] ERROR: Could not update " + dst);
    return false;
}

namespace auth {
    constexpr const wchar_t* kHost = L"licensing-backend.donghiem114.workers.dev";
    constexpr const wchar_t* kUserAgent = L"GZ-Account-Loader";
    constexpr const wchar_t* kLoginPath = L"/loader/login";
    constexpr const wchar_t* kMePath = L"/loader/me";
    constexpr const wchar_t* kActivatePath = L"/loader/keys/activate";
    constexpr int kSignatureVersion = 2;

    struct Session {
        std::string token;
        std::string username;
        std::string key;
        std::string expires_at;
        long long remaining_seconds = 0;
        bool active = false;
    };

    std::string local_appdata_root() {
        char buffer[4096] = {};
        DWORD len = GetEnvironmentVariableA("LOCALAPPDATA", buffer, (DWORD)sizeof(buffer));
        std::string root = (len > 0 && len < sizeof(buffer)) ? std::string(buffer, len) : ".";
        root += "\\UnifiedTelemetryNode";
        return root;
    }

    std::string session_path() {
        return local_appdata_root() + "\\loader_session.json";
    }

    void delete_persisted_auth_files() {
        DeleteFileA(session_path().c_str());
        DeleteFileA((local_appdata_root() + "\\key.txt").c_str());
        DeleteFileA("loader_session.json");
        DeleteFileA("key.txt");
    }

    std::string json_string(const nlohmann::json& json, const char* field) {
        auto it = json.find(field);
        return it != json.end() && it->is_string() ? it->get<std::string>() : "";
    }

    long long json_int64(const nlohmann::json& json, const char* field) {
        auto it = json.find(field);
        if (it == json.end()) return 0;
        if (it->is_number_integer()) return it->get<long long>();
        if (it->is_number_unsigned()) return (long long)it->get<unsigned long long>();
        return 0;
    }

    std::wstring utf8_to_wide(const std::string& value) {
        if (value.empty()) return L"";
        int required = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
        if (required <= 0) return L"";
        std::wstring out((size_t)required - 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, out.data(), required);
        return out;
    }

    std::string generate_nonce() {
        BYTE bytes[32] = {};
        HCRYPTPROV hCrypt = 0;
        if (CryptAcquireContextA(&hCrypt, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
            CryptGenRandom(hCrypt, sizeof(bytes), bytes);
            CryptReleaseContext(hCrypt, 0);
        } else {
            LARGE_INTEGER perf;
            QueryPerformanceCounter(&perf);
            unsigned long long seed = (unsigned long long)perf.QuadPart ^ GetTickCount64();
            for (BYTE& b : bytes) {
                seed ^= seed << 13;
                seed ^= seed >> 7;
                seed ^= seed << 17;
                b = (BYTE)(seed & 0xFF);
            }
        }

        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (BYTE b : bytes) oss << std::setw(2) << (int)b;
        return oss.str();
    }

    std::string trim(std::string value) {
        const char* ws = " \t\r\n";
        size_t first = value.find_first_not_of(ws);
        if (first == std::string::npos) return "";
        size_t last = value.find_last_not_of(ws);
        return value.substr(first, last - first + 1);
    }

    std::string get_hwid() {
        std::string raw;
        DWORD vol = 0;
        if (GetVolumeInformationA("C:\\", NULL, 0, &vol, NULL, NULL, NULL, 0)) {
            char vol_str[32] = {};
            sprintf_s(vol_str, "%08X", vol);
            raw += vol_str;
        } else {
            raw += "NOVOL";
        }

        HANDLE hDrive = CreateFileA("\\\\.\\PhysicalDrive0", 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDrive != INVALID_HANDLE_VALUE) {
            STORAGE_PROPERTY_QUERY query = {};
            query.PropertyId = StorageDeviceProperty;
            query.QueryType = PropertyStandardQuery;
            STORAGE_DESCRIPTOR_HEADER header = {};
            DWORD bytes = 0;
            if (DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &header, sizeof(header), &bytes, NULL) && header.Size) {
                std::vector<BYTE> buffer(header.Size);
                if (DeviceIoControl(hDrive, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), buffer.data(), (DWORD)buffer.size(), &bytes, NULL)) {
                    auto* desc = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(buffer.data());
                    if (desc->SerialNumberOffset && desc->SerialNumberOffset < buffer.size()) {
                        raw += trim(reinterpret_cast<const char*>(buffer.data() + desc->SerialNumberOffset));
                    }
                }
            }
            CloseHandle(hDrive);
        } else {
            raw += "NODISK";
        }

        int cpuinfo[4] = {};
        __cpuid(cpuinfo, 0x80000002);
        char cpu_str[64] = {};
        sprintf_s(cpu_str, "%08X%08X%08X%08X", cpuinfo[0], cpuinfo[1], cpuinfo[2], cpuinfo[3]);
        raw += cpu_str;

        HKEY hKey = nullptr;
        char bios_serial[256] = {};
        DWORD bios_len = sizeof(bios_serial);
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExA(hKey, "BaseBoardSerialNumber", NULL, NULL, (LPBYTE)bios_serial, &bios_len) == ERROR_SUCCESS) {
                raw += trim(std::string(bios_serial));
            } else {
                raw += "NOBIOS";
            }
            RegCloseKey(hKey);
        } else {
            raw += "NOREG";
        }

        if (raw.empty()) raw = "UNKNOWN";
        return Sha::hmac_sha256("GZ_HARD_HWID_FINAL_V2", raw).substr(0, 16);
    }

    bool http_json(const wchar_t* method, const wchar_t* path, const std::string& body, const std::string& token, std::string& response) {
        response.clear();
        HINTERNET hSession = WinHttpOpen(kUserAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) return false;
        HINTERNET hConnect = WinHttpConnect(hSession, kHost, INTERNET_DEFAULT_HTTPS_PORT, 0);
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

        std::wstring headers = L"Content-Type: application/json\r\nUser-Agent: GZ-Account-Loader\r\n";
        if (!token.empty()) {
            headers += L"Authorization: Bearer ";
            headers += utf8_to_wide(token);
            headers += L"\r\n";
        }

        LPVOID requestBody = body.empty() ? WINHTTP_NO_REQUEST_DATA : (LPVOID)body.c_str();
        DWORD requestBodyLength = body.empty() ? 0 : (DWORD)body.length();
        bool ok = WinHttpSendRequest(hRequest, headers.c_str(), -1, requestBody, requestBodyLength, requestBodyLength, 0) == TRUE;
        if (ok) ok = WinHttpReceiveResponse(hRequest, NULL) == TRUE;
        if (ok) {
            DWORD size = 0;
            do {
                size = 0;
                if (!WinHttpQueryDataAvailable(hRequest, &size) || size == 0) break;
                std::vector<char> buffer(size + 1, 0);
                DWORD downloaded = 0;
                if (WinHttpReadData(hRequest, buffer.data(), size, &downloaded)) {
                    response.append(buffer.data(), downloaded);
                }
            } while (size > 0);
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return ok && !response.empty();
    }

    bool post_json(const wchar_t* path, const nlohmann::json& body, const std::string& token, std::string& response) {
        return http_json(L"POST", path, body.dump(), token, response);
    }

    bool get_json(const std::wstring& path, const std::string& token, std::string& response) {
        return http_json(L"GET", path.c_str(), "", token, response);
    }

    Session load_session() {
        delete_persisted_auth_files();
        return {};
    }

    void save_session(const Session& session) {
        (void)session;
    }

    bool apply_entitlement(const nlohmann::json& response, Session& session) {
        auto it = response.find("entitlement");
        if (it == response.end() || !it->is_object()) {
            session.active = false;
            session.key.clear();
            session.expires_at.clear();
            session.remaining_seconds = 0;
            return false;
        }

        session.key = json_string(*it, "key");
        session.expires_at = json_string(*it, "expires_at");
        session.remaining_seconds = json_int64(*it, "remaining_seconds");
        session.active = it->value("active", false) && session.remaining_seconds > 0;
        return session.active;
    }

    bool apply_account_response(const nlohmann::json& json, Session& session, const std::string& fallbackUsername) {
        std::string token = json_string(json, "token");
        if (!token.empty()) session.token = token;

        auto userIt = json.find("user");
        session.username = userIt != json.end() && userIt->is_object()
            ? json_string(*userIt, "username")
            : fallbackUsername;

        return apply_entitlement(json, session);
    }

    bool refresh_entitlement(Session& session, const std::string& hwid) {
        if (session.token.empty()) return false;

        std::string response;
        std::wstring path = std::wstring(kMePath) + L"?hwid=" + utf8_to_wide(hwid);
        if (!get_json(path, session.token, response)) {
            return false;
        }

        nlohmann::json json = nlohmann::json::parse(response, nullptr, false);
        if (!json.is_object()) {
            return false;
        }

        return apply_account_response(json, session, session.username);
    }

    bool resume_session(const std::string& hwid, Session& session) {
        (void)hwid;
        session = load_session();
        return false;
    }

    enum : int {
        kIdUserEdit = 1001,
        kIdPasswordEdit = 1002,
        kIdKeyEdit = 1003,
        kIdOk = 1004,
        kIdCancel = 1005,
        kIdRedeem = 1008,
        kIdContinue = 1009
    };

    struct AuthDialogState {
        bool key_mode = false;
        bool done = false;
        bool ok = false;
        HWND user_edit = nullptr;
        HWND password_edit = nullptr;
        HWND key_edit = nullptr;
        std::string username;
        std::string password;
        std::string key;
    };

    std::string window_text(HWND hwnd) {
        const int len = GetWindowTextLengthA(hwnd);
        if (len <= 0) return "";
        std::string text((size_t)len + 1, '\0');
        GetWindowTextA(hwnd, text.data(), len + 1);
        text.resize((size_t)len);
        return text;
    }

    void set_default_font(HWND hwnd) {
        SendMessageA(hwnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    }

    HWND create_label(HWND parent, const char* text, int x, int y, int w, int h) {
        HWND hwnd = CreateWindowExA(0, "STATIC", text, WS_CHILD | WS_VISIBLE,
            x, y, w, h, parent, nullptr, GetModuleHandleA(nullptr), nullptr);
        set_default_font(hwnd);
        return hwnd;
    }

    HWND create_edit(HWND parent, int id, int x, int y, int w, int h, bool password) {
        DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL;
        if (password) style |= ES_PASSWORD;
        HWND hwnd = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", style,
            x, y, w, h, parent, (HMENU)(INT_PTR)id, GetModuleHandleA(nullptr), nullptr);
        set_default_font(hwnd);
        return hwnd;
    }

    HWND create_button(HWND parent, int id, const char* text, int x, int y, int w, int h) {
        HWND hwnd = CreateWindowExA(0, "BUTTON", text, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            x, y, w, h, parent, (HMENU)(INT_PTR)id, GetModuleHandleA(nullptr), nullptr);
        set_default_font(hwnd);
        return hwnd;
    }

    LRESULT CALLBACK auth_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        AuthDialogState* state = reinterpret_cast<AuthDialogState*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));

        if (msg == WM_NCCREATE) {
            auto* create = reinterpret_cast<CREATESTRUCTA*>(lparam);
            state = reinterpret_cast<AuthDialogState*>(create->lpCreateParams);
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
            return TRUE;
        }

        switch (msg) {
        case WM_CREATE:
            if (!state) return 0;
            if (state->key_mode) {
                create_label(hwnd, "Nhap key", 24, 24, 260, 20);
                state->key_edit = create_edit(hwnd, kIdKeyEdit, 24, 48, 336, 24, false);
                create_button(hwnd, kIdOk, "Xac thuc", 188, 92, 82, 28);
                create_button(hwnd, kIdCancel, "Huy", 278, 92, 82, 28);
                SetFocus(state->key_edit);
            } else {
                create_label(hwnd, "Tai khoan", 24, 24, 120, 20);
                state->user_edit = create_edit(hwnd, kIdUserEdit, 128, 22, 232, 24, false);
                create_label(hwnd, "Mat khau", 24, 60, 120, 20);
                state->password_edit = create_edit(hwnd, kIdPasswordEdit, 128, 58, 232, 24, true);
                create_button(hwnd, kIdOk, "Dang nhap", 188, 104, 82, 28);
                create_button(hwnd, kIdCancel, "Huy", 278, 104, 82, 28);
                SetFocus(state->user_edit);
            }
            return 0;

        case WM_COMMAND:
            if (!state) return 0;
            switch (LOWORD(wparam)) {
            case kIdOk:
                if (state->key_mode) {
                    state->key = window_text(state->key_edit);
                    if (state->key.empty()) {
                        MessageBoxA(hwnd, "Vui long nhap key.", "GZ Loader", MB_ICONWARNING | MB_OK);
                        return 0;
                    }
                } else {
                    state->username = window_text(state->user_edit);
                    state->password = window_text(state->password_edit);
                    if (state->username.empty() || state->password.empty()) {
                        MessageBoxA(hwnd, "Vui long nhap tai khoan va mat khau.", "GZ Loader", MB_ICONWARNING | MB_OK);
                        return 0;
                    }
                }
                state->ok = true;
                state->done = true;
                DestroyWindow(hwnd);
                return 0;

            case kIdCancel:
                state->ok = false;
                state->done = true;
                DestroyWindow(hwnd);
                return 0;
            }
            return 0;

        case WM_CLOSE:
            if (state) {
                state->ok = false;
                state->done = true;
            }
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            if (state) state->done = true;
            return 0;
        }

        return DefWindowProcA(hwnd, msg, wparam, lparam);
    }

    bool show_auth_dialog(AuthDialogState& state) {
        const char* class_name = "GzLoaderAuthWindow";
        static bool registered = false;
        if (!registered) {
            WNDCLASSEXA wc = {};
            wc.cbSize = sizeof(wc);
            wc.lpfnWndProc = auth_window_proc;
            wc.hInstance = GetModuleHandleA(nullptr);
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
            wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            wc.lpszClassName = class_name;
            RegisterClassExA(&wc);
            registered = true;
        }

        const int width = 400;
        const int height = state.key_mode ? 170 : 185;
        const int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
        const int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
        HWND hwnd = CreateWindowExA(WS_EX_DLGMODALFRAME,
            class_name,
            state.key_mode ? "GZ Loader - Key" : "GZ Loader - Dang nhap",
            WS_CAPTION | WS_SYSMENU,
            x, y, width, height,
            nullptr, nullptr, GetModuleHandleA(nullptr), &state);
        if (!hwnd) return false;

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        MSG msg = {};
        while (!state.done && GetMessageA(&msg, nullptr, 0, 0) > 0) {
            if (!IsDialogMessageA(hwnd, &msg)) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }

        return state.ok;
    }

    struct StatusDialogState {
        const Session* session = nullptr;
        bool done = false;
        int action = 0; // 1 = continue, 2 = redeem key
    };

    LRESULT CALLBACK status_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        StatusDialogState* state = reinterpret_cast<StatusDialogState*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));

        if (msg == WM_NCCREATE) {
            auto* create = reinterpret_cast<CREATESTRUCTA*>(lparam);
            state = reinterpret_cast<StatusDialogState*>(create->lpCreateParams);
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
            return TRUE;
        }

        switch (msg) {
        case WM_CREATE:
            if (!state || !state->session) return 0;
            create_label(hwnd, "Tai khoan con han", 24, 22, 336, 20);
            create_label(hwnd, ("Tai khoan: " + state->session->username).c_str(), 24, 52, 336, 20);
            create_label(hwnd, ("Het han: " + (state->session->expires_at.empty() ? std::string("ACTIVE") : state->session->expires_at)).c_str(), 24, 78, 336, 20);
            create_button(hwnd, kIdContinue, "Tiep tuc", 88, 122, 92, 30);
            create_button(hwnd, kIdRedeem, "Nap them key", 188, 122, 104, 30);
            SetFocus(GetDlgItem(hwnd, kIdContinue));
            return 0;

        case WM_COMMAND:
            if (!state) return 0;
            if (LOWORD(wparam) == kIdContinue) {
                state->action = 1;
                state->done = true;
                DestroyWindow(hwnd);
                return 0;
            }
            if (LOWORD(wparam) == kIdRedeem) {
                state->action = 2;
                state->done = true;
                DestroyWindow(hwnd);
                return 0;
            }
            return 0;

        case WM_CLOSE:
            if (state) {
                state->action = 0;
                state->done = true;
            }
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            if (state) state->done = true;
            return 0;
        }

        return DefWindowProcA(hwnd, msg, wparam, lparam);
    }

    int show_status_dialog(const Session& session) {
        const char* class_name = "GzLoaderStatusWindow";
        static bool registered = false;
        if (!registered) {
            WNDCLASSEXA wc = {};
            wc.cbSize = sizeof(wc);
            wc.lpfnWndProc = status_window_proc;
            wc.hInstance = GetModuleHandleA(nullptr);
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
            wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            wc.lpszClassName = class_name;
            RegisterClassExA(&wc);
            registered = true;
        }

        StatusDialogState state;
        state.session = &session;
        const int width = 400;
        const int height = 205;
        const int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
        const int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
        HWND hwnd = CreateWindowExA(WS_EX_DLGMODALFRAME,
            class_name,
            "GZ Loader - License",
            WS_CAPTION | WS_SYSMENU,
            x, y, width, height,
            nullptr, nullptr, GetModuleHandleA(nullptr), &state);
        if (!hwnd) return 0;

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        MSG msg = {};
        while (!state.done && GetMessageA(&msg, nullptr, 0, 0) > 0) {
            if (!IsDialogMessageA(hwnd, &msg)) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }

        return state.action;
    }

    bool login(Session& session, const std::string& hwid) {
        AuthDialogState input;
        if (!show_auth_dialog(input)) return false;

        nlohmann::json request;
        request["username"] = input.username;
        request["password"] = input.password;
        request["hwid"] = hwid;

        std::string response;
        if (!post_json(kLoginPath, request, "", response)) {
            MessageBoxA(nullptr, "Khong ket noi duoc may chu tai khoan.", "GZ Loader", MB_ICONERROR | MB_OK);
            return false;
        }

        nlohmann::json json = nlohmann::json::parse(response, nullptr, false);
        if (!json.is_object() || !json.contains("token")) {
            MessageBoxA(nullptr, response.c_str(), "Dang nhap that bai", MB_ICONERROR | MB_OK);
            return false;
        }

        apply_account_response(json, session, input.username);
        refresh_entitlement(session, hwid);
        return true;
    }

    bool activate_key(Session& session, const std::string& hwid) {
        AuthDialogState input;
        input.key_mode = true;
        if (!show_auth_dialog(input)) return false;

        nlohmann::json request;
        request["key"] = input.key;
        request["hwid"] = hwid;
        request["nonce"] = generate_nonce();
        request["signature_version"] = kSignatureVersion;

        std::string response;
        if (!post_json(kActivatePath, request, session.token, response)) {
            MessageBoxA(nullptr, "Khong ket noi duoc may chu key.", "GZ Loader", MB_ICONERROR | MB_OK);
            return false;
        }

        nlohmann::json json = nlohmann::json::parse(response, nullptr, false);
        if (!json.is_object() || json_string(json, "status") != "ACTIVATED") {
            MessageBoxA(nullptr, response.c_str(), "Kich hoat key that bai", MB_ICONERROR | MB_OK);
            return false;
        }

        refresh_entitlement(session, hwid);
        if (!session.active) {
            session.key = input.key;
            session.active = true;
        }
        return true;
    }

    bool authenticate() {
        delete_persisted_auth_files();
        const std::string hwid = get_hwid();
        Session session;

        if (resume_session(hwid, session)) {
            return true;
        }

        if (!login(session, hwid)) return false;

        if (session.active) {
            const int action = show_status_dialog(session);
            if (action == 1) return true;
            if (action != 2) return false;
            if (!activate_key(session, hwid)) return false;
            refresh_entitlement(session, hwid);
            return session.active;
        }

        MessageBoxA(nullptr, "Tai khoan chua co thoi gian su dung. Vui long nhap key de kich hoat.", "GZ Loader", MB_ICONINFORMATION | MB_OK);
        if (!activate_key(session, hwid)) return false;

        return true;
    }
}

int main() {
    protec::scan_detection_time = 1000;
#ifdef _DEBUG
    const auto hardening = protec::apply_baseline_hardening();
    print_log(std::string("[PROTEC] baseline heap=") +
        (hardening.heap_termination_enabled ? "ok" : "fail") +
        " dll-search=" +
        (hardening.dll_search_order_hardened ? "ok" : "off"));
#else
    protec::apply_baseline_hardening();
#endif
    protec::start_protect(false);

    if (!is_admin()) {
        run_as_admin();
        return 0;
    }

    if (!auth::authenticate()) {
        MessageBoxA(NULL, "Xac thuc loader that bai.", "GZ Loader", MB_ICONERROR | MB_OK);
        return 1;
    }

    std::string drive_str = "Z:";
    system(("mountvol " + drive_str + " /D >nul 2>&1").c_str());
    Sleep(500);

#ifdef _DEBUG
    print_log("[*] Mounting EFI to " + drive_str + "...");
#endif
    system(("mountvol " + drive_str + " /S").c_str());
    Sleep(1000);

    if (GetFileAttributesA(drive_str.c_str()) == INVALID_FILE_ATTRIBUTES) {
        print_log("[-] ERROR: Failed to mount EFI partition.");
        return 1;
    }

    // Temporary extraction paths
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    std::string efiTemp = std::string(tempPath) + "uefi_boot_temp.efi";
    std::string dllTemp = std::string(tempPath) + "attach_temp.dll";

#ifdef _DEBUG
    print_log("[*] Extracting embedded resources...");
#endif
    if (!extract_resource(IDR_EFI_BOOT, efiTemp) || !extract_resource(IDR_ATTACH_DLL, dllTemp)) {
        print_log("[-] ERROR: Failed to extract embedded components.");
        return 1;
    }
    
    // Resource extraction complete, safe to wipe headers now for stealth
    protec::erase_pe_header();

    std::string ms_dir  = drive_str + "\\EFI\\Microsoft\\Boot";
    std::string ms_boot = ms_dir + "\\bootmgfw.efi";
    std::string ms_orig = ms_dir + "\\bootmgfw.original.efi";
    std::string ms_dll  = ms_dir + "\\hyperv-attachment.dll";

    std::string fb_dir  = drive_str + "\\EFI\\Boot";
    std::string fb_boot = fb_dir + "\\bootx64.efi";
    std::string fb_orig = fb_dir + "\\bootx64.original.efi";

    SHCreateDirectoryExA(NULL, ms_dir.c_str(), NULL);
    SHCreateDirectoryExA(NULL, fb_dir.c_str(), NULL);

    // Backup & Copy Path 1
    if (GetFileAttributesA(ms_orig.c_str()) == INVALID_FILE_ATTRIBUTES) {
        MoveFileA(ms_boot.c_str(), ms_orig.c_str());
    }
    aggressive_copy(efiTemp, ms_boot);
    aggressive_copy(dllTemp, ms_dll);

    // Backup & Copy Path 2
    if (GetFileAttributesA(fb_boot.c_str()) != INVALID_FILE_ATTRIBUTES) {
        if (GetFileAttributesA(fb_orig.c_str()) == INVALID_FILE_ATTRIBUTES) {
            MoveFileA(fb_boot.c_str(), fb_orig.c_str());
        }
        aggressive_copy(efiTemp, fb_boot);
    } else {
        aggressive_copy(efiTemp, fb_boot);
    }

    system("bcdedit /set hypervisorlaunchtype auto >nul 2>&1");
    // No user-facing logs in Release mode for total silence
#ifdef _DEBUG
    print_log("[+] SUCCESS: UEFI BOOT REGISTRY UPDATED.");
    print_log("[!] RESTARTING SYSTEM IN 5 SECONDS...");
#endif

    // [USER-REQUESTED] Skip cleanup in loader to maintain focus on loading
    /*
    DeleteFileA(efiTemp.c_str());
    DeleteFileA(dllTemp.c_str());
    system(("mountvol " + drive_str + " /D >nul 2>&1").c_str());
    */

    // FORCE REBOOT: /r = reboot, /t 5 = 5 seconds delay, /f = force close apps
    system("shutdown /r /t 5 /f");
    
    return 0;
}
