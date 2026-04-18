#include <windows.h>
#include <iostream>
#include <string>
#include <shlobj.h> // Để dùng SHCreateDirectoryExA

#pragma comment(lib, "Shell32.lib")

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
        std::cout << "[+] SUCCESS: Updated " << dst << std::endl;
        return true;
    }
    std::cout << "[-] FAILED: Could not update " << dst << " (Error: " << GetLastError() << ")" << std::endl;
    return false;
}

void verify_with_api(const std::string& path) {
    WIN32_FIND_DATAA data;
    HANDLE h = FindFirstFileA(path.c_str(), &data);
    if (h != INVALID_HANDLE_VALUE) {
        std::cout << "[VERIFIED] " << data.cFileName << " | Size: " << data.nFileSizeLow << " bytes" << std::endl;
        FindClose(h);
    } else {
        std::cout << "[NOT FOUND] File error: " << path << " (Error: " << GetLastError() << ")" << std::endl;
    }
}

int main() {
    if (!is_admin()) {
        run_as_admin();
        return 0;
    }

    // Ổ B cố định theo yêu cầu
    std::string drive_str = "B:";

    std::cout << "[*] Cleaning up drive " << drive_str << " (if exists)..." << std::endl;
    system(("mountvol " + drive_str + " /D >nul 2>&1").c_str());
    Sleep(500);

    std::cout << "[*] Mounting EFI to " << drive_str << "..." << std::endl;
    system(("mountvol " + drive_str + " /S").c_str());
    Sleep(1000);

    // Kiểm tra xem ổ đã thực sự được mount chưa
    if (GetFileAttributesA(drive_str.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::cout << "[-] ERROR: Failed to mount EFI partition to " << drive_str << "." << std::endl;
        std::cout << "[!] Hay chac chan rang o B: khong bi chiem dung boi thiet bi khac." << std::endl;
        system("pause");
        return 1;
    }

    char current_dir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, current_dir);
    std::string bin_path(current_dir);
    
    std::string src_efi = bin_path + "\\uefi-boot.efi";
    std::string src_dll = bin_path + "\\hyperv-attachment.dll";

    std::string ms_dir  = drive_str + "\\EFI\\Microsoft\\Boot";
    std::string ms_boot = ms_dir + "\\bootmgfw.efi";
    std::string ms_orig = ms_dir + "\\bootmgfw.original.efi";
    std::string ms_dll  = ms_dir + "\\hyperv-attachment.dll";

    std::string fb_dir  = drive_str + "\\EFI\\Boot";
    std::string fb_boot = fb_dir + "\\bootx64.efi";
    std::string fb_orig = fb_dir + "\\bootx64.original.efi";

    // Đảm bảo cấu trúc thư mục tồn tại
    SHCreateDirectoryExA(NULL, ms_dir.c_str(), NULL);
    SHCreateDirectoryExA(NULL, fb_dir.c_str(), NULL);

    // Backup & Copy Path 1
    if (GetFileAttributesA(ms_orig.c_str()) == INVALID_FILE_ATTRIBUTES) {
        MoveFileA(ms_boot.c_str(), ms_orig.c_str());
    }
    aggressive_copy(src_efi, ms_boot);
    aggressive_copy(src_dll, ms_dll);

    // Backup & Copy Path 2
    if (GetFileAttributesA(fb_boot.c_str()) != INVALID_FILE_ATTRIBUTES) {
        if (GetFileAttributesA(fb_orig.c_str()) == INVALID_FILE_ATTRIBUTES) {
            MoveFileA(fb_boot.c_str(), fb_orig.c_str());
        }
        aggressive_copy(src_efi, fb_boot);
    } else {
        aggressive_copy(src_efi, fb_boot);
    }

    system("bcdedit /set hypervisorlaunchtype auto >nul 2>&1");
    system("bcdedit /set {fwbootmgr} displayorder {bootmgr} /addfirst >nul 2>&1");

    std::cout << "\n[OK] BOOT UPDATED. Verifying Result:" << std::endl;
    verify_with_api(ms_boot);
    verify_with_api(ms_dll);
    verify_with_api(fb_boot);
    
    std::cout << "\n[!] RESTART NOW. Press any key to unmount and exit..." << std::endl;
    system("pause >nul");

    system(("mountvol " + drive_str + " /D >nul 2>&1").c_str());
    return 0;
}
