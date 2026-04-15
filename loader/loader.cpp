#include <windows.h>
#include <iostream>
#include <string>

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
    system(("attrib -s -h -r " + dst + " >nul 2>&1").c_str());
    DeleteFileA(dst.c_str());
    if (CopyFileA(src.c_str(), dst.c_str(), FALSE)) {
        std::cout << "[+] SUCCESS: Updated " << dst << std::endl;
        return true;
    }
    return false;
}

int main() {
    if (!is_admin()) {
        run_as_admin();
        return 0;
    }

    std::cout << "========================================" << std::endl;
    std::cout << "    hyper-reV MULTI-BOOT LOADER v3.0    " << std::endl;
    std::cout << "========================================" << std::endl;

    system("mountvol Z: /S >nul 2>&1");
    Sleep(500);

    char current_dir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, current_dir);
    std::string bin_path(current_dir);
    std::string src_efi = bin_path + "\\uefi-boot.efi";
    std::string src_dll = bin_path + "\\hyperv-attachment.dll";

    // Path 1: Microsoft Bootloader
    std::string ms_boot = "Z:\\EFI\\Microsoft\\Boot\\bootmgfw.efi";
    std::string ms_orig = "Z:\\EFI\\Microsoft\\Boot\\bootmgfw.original.efi";
    
    // Path 2: Fallback Bootloader (Where your BIOS is likely booting from)
    std::string fb_boot = "Z:\\EFI\\Boot\\bootx64.efi";
    std::string fb_orig = "Z:\\EFI\\Boot\\bootx64.original.efi";

    // Backup and overwrite Path 1
    if (GetFileAttributesA(ms_orig.c_str()) == INVALID_FILE_ATTRIBUTES) {
        system(("attrib -s -h -r " + ms_boot + " >nul 2>&1").c_str());
        MoveFileA(ms_boot.c_str(), ms_orig.c_str());
    }
    aggressive_copy(src_efi, ms_boot);
    aggressive_copy(src_dll, "Z:\\EFI\\Microsoft\\Boot\\hyperv-attachment.dll");

    // Backup and overwrite Path 2
    if (GetFileAttributesA(fb_boot.c_str()) != INVALID_FILE_ATTRIBUTES) {
        if (GetFileAttributesA(fb_orig.c_str()) == INVALID_FILE_ATTRIBUTES) {
            std::cout << "[*] Backing up fallback bootloader..." << std::endl;
            system(("attrib -s -h -r " + fb_boot + " >nul 2>&1").c_str());
            MoveFileA(fb_boot.c_str(), fb_orig.c_str());
        }
        aggressive_copy(src_efi, fb_boot);
    } else {
        // If bootx64.efi doesn't exist, we create it just in case
        std::cout << "[*] Creating fallback bootloader..." << std::endl;
        CreateDirectoryA("Z:\\EFI\\Boot", NULL);
        aggressive_copy(src_efi, fb_boot);
    }

    system("bcdedit /set hypervisorlaunchtype auto >nul 2>&1");
    std::cout << "\n[+++] ALL BOOT PATHS UPDATED [+++]" << std::endl;
    std::cout << "[!] RESTART NOW to see your PHAOHACKGAME logo!" << std::endl;

    system("pause");
    return 0;
}
