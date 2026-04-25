#include <windows.h>
#include <iostream>
#include <string>
#include <shlobj.h>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <vector>
#include "resource.h"
#include "../PUBG-2/pubg/sdk/protector/protector.hpp"

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "User32.lib")

// --- LINKER CONFIG: WINDOWS FOR RELEASE, CONSOLE FOR DEBUG ---
#ifdef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#else
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

void print_log(const std::string& msg) {
#ifdef _DEBUG
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm bt;
    localtime_s(&bt, &in_time_t);
    std::cout << "[" << std::put_time(&bt, "%H:%M:%S") << "] " << msg << std::endl;
#else
    // Pure silence in Release mode
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

int main() {
    protector::start_protect();

    if (!is_admin()) {
        run_as_admin();
        return 0;
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
