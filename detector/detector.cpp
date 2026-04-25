#define NOMINMAX
#include <Windows.h>
#include <TlHelp32.h>
#include <winver.h>
#include "../protec/protector.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace
{
    struct ioc_string_t
    {
        const char* label;
        const char* value;
        int score;
    };

    struct efi_file_ioc_t
    {
        const wchar_t* relative_path;
        const char* label;
        int score;
    };

    struct finding_t
    {
        std::string category;
        std::wstring target;
        std::string ioc_label;
        int score;
        std::string detail;
    };

    struct process_report_t
    {
        DWORD pid = 0;
        std::wstring name;
        std::wstring path;
        bool elevated = false;
        std::vector<finding_t> findings;
        std::size_t scanned_bytes = 0;
        int score = 0;
    };

    struct system_report_t
    {
        bool is_admin = false;
        bool debugger_present = false;
        bool efi_scan_attempted = false;
        bool efi_scan_succeeded = false;
        bool b_drive_scan_attempted = false;
        bool b_drive_scan_succeeded = false;
        protec::hardening_result_t hardening = {};
        int total_score = 0;
        std::vector<finding_t> efi_findings;
        std::vector<finding_t> b_drive_findings;
        std::vector<process_report_t> process_reports;
    };

    constexpr std::size_t kMaxProcessScanBytes = 48ull * 1024ull * 1024ull;
    constexpr std::size_t kMaxImageRegionScanBytes = 8ull * 1024ull * 1024ull;
    constexpr std::size_t kMaxPrivateRegionScanBytes = 1ull * 1024ull * 1024ull;

    const std::array<ioc_string_t, 18> kProcessIocs = {{
        {"loader_banner", "hyper-reV Loader (GUI-lite)", 30},
        {"embedded_payloads", "This executable contains embedded payloads.", 25},
        {"deploy_dll", "Deploying embedded hyperv-attachment.dll...", 25},
        {"bcdedit_auto", "bcdedit /set hypervisorlaunchtype auto", 20},
        {"efi_path", "\\EFI\\Microsoft\\Boot", 20},
        {"boot_backup", "bootmgfw.original.efi", 20},
        {"deploy_note", "load-hyper-reV.txt", 20},
        {"usermode_start", "Starting Usermode...", 15},
        {"usermode_setup", "Setting up system...", 15},
        {"usermode_ready", "Ready!", 10},
        {"hv_not_loaded", "hyperv-attachment doesn't seem to be loaded", 25},
        {"guest_phys_read", "reads memory from a given guest physical address", 20},
        {"guest_phys_write", "writes memory to a given guest physical address", 20},
        {"hide_guest_page", "hide a physical page's real contents from the guest", 25},
        {"flush_logs", "flush trap frame logs from hooks", 20},
        {"heap_free_pages", "get hyperv-attachment's heap free page count", 25},
        {"slat_hook_help", "add a hook on specified kernel code", 20},
        {"hv_attachment_name", "hyperv-attachment.dll", 15},
    }};

    const std::array<ioc_string_t, 2> kMarkerIocs = {{
        {"deploy_marker", "DEPLOYED BY PHAOHACKGAME", 40},
        {"version_marker", "VER_9999_MARKER", 40},
    }};

    const std::array<efi_file_ioc_t, 5> kEfiFileIocs = {{
        {L"EFI\\Microsoft\\Boot\\uefi-boot.efi", "efi_uefi_boot", 60},
        {L"EFI\\Microsoft\\Boot\\loader.exe", "efi_loader", 60},
        {L"EFI\\Microsoft\\Boot\\load-hyper-reV.txt", "efi_deploy_note", 60},
        {L"EFI\\Microsoft\\Boot\\bootmgfw.original.efi", "efi_boot_backup", 40},
        {L"EFI\\Microsoft\\Boot\\hyperv-attachment.dll", "efi_attachment", 40},
    }};

    const std::array<ioc_string_t, 15> kCapabilityIocs = {{
        {"efi_boot_path", "\\EFI\\Microsoft\\Boot", 12},
        {"boot_file_name", "bootmgfw.efi", 10},
        {"bcdedit_setting", "hypervisorlaunchtype", 12},
        {"mountvol_usage", "mountvol", 12},
        {"runas_verb", "runas", 6},
        {"copy_file", "CopyFileW", 4},
        {"move_file", "MoveFileExW", 4},
        {"delete_file", "DeleteFileW", 4},
        {"write_file", "WriteFile", 4},
        {"shell_execute", "ShellExecuteW", 6},
        {"create_process", "CreateProcessW", 6},
        {"query_system_info", "NtQuerySystemInformation", 6},
        {"adjust_privilege", "RtlAdjustPrivilege", 6},
        {"virtual_lock", "VirtualLock", 6},
        {"process_snapshot", "CreateToolhelp32Snapshot", 6},
    }};

    const std::array<std::wstring_view, 3> kStandardEfiExecutables = {{
        L"bootmgfw.efi",
        L"bootmgr.efi",
        L"memtest.efi",
    }};

    std::string narrow_utf8(const std::wstring& input)
    {
        if (input.empty())
        {
            return {};
        }

        const int bytes_needed = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), static_cast<int>(input.size()), nullptr, 0, nullptr, nullptr);

        if (bytes_needed <= 0)
        {
            return {};
        }

        std::string output(static_cast<std::size_t>(bytes_needed), '\0');
        WideCharToMultiByte(CP_UTF8, 0, input.c_str(), static_cast<int>(input.size()), output.data(), bytes_needed, nullptr, nullptr);
        return output;
    }

    std::wstring to_lower_copy(std::wstring value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](const wchar_t ch)
        {
            return static_cast<wchar_t>(towlower(ch));
        });

        return value;
    }

    bool starts_with_case_insensitive(const std::wstring& value, const std::wstring& prefix)
    {
        if (value.size() < prefix.size())
        {
            return false;
        }

        return to_lower_copy(value.substr(0, prefix.size())) == to_lower_copy(prefix);
    }

    bool is_process_readable(const DWORD protection)
    {
        if ((protection & PAGE_GUARD) != 0 || (protection & PAGE_NOACCESS) != 0)
        {
            return false;
        }

        switch (protection & 0xff)
        {
        case PAGE_READONLY:
        case PAGE_READWRITE:
        case PAGE_WRITECOPY:
        case PAGE_EXECUTE_READ:
        case PAGE_EXECUTE_READWRITE:
        case PAGE_EXECUTE_WRITECOPY:
            return true;
        default:
            return false;
        }
    }

    bool run_hidden_command(const std::wstring& command_line)
    {
        STARTUPINFOW startup_info = {};
        startup_info.cb = sizeof(startup_info);
        startup_info.dwFlags = STARTF_USESHOWWINDOW;
        startup_info.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION process_info = {};

        std::vector<wchar_t> mutable_command(command_line.begin(), command_line.end());
        mutable_command.push_back(L'\0');

        if (!CreateProcessW(nullptr, mutable_command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &startup_info, &process_info))
        {
            return false;
        }

        WaitForSingleObject(process_info.hProcess, INFINITE);

        DWORD exit_code = 1;
        GetExitCodeProcess(process_info.hProcess, &exit_code);

        CloseHandle(process_info.hThread);
        CloseHandle(process_info.hProcess);

        return exit_code == 0;
    }

    bool is_running_as_admin()
    {
        BOOL is_admin = FALSE;
        SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;
        PSID administrators_group = nullptr;

        if (!AllocateAndInitializeSid(
            &nt_authority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &administrators_group))
        {
            return false;
        }

        if (!CheckTokenMembership(nullptr, administrators_group, &is_admin))
        {
            is_admin = FALSE;
        }

        FreeSid(administrators_group);
        return is_admin == TRUE;
    }

    std::optional<std::wstring> choose_free_drive_letter()
    {
        const DWORD drives = GetLogicalDrives();

        for (wchar_t letter = L'Z'; letter >= L'R'; --letter)
        {
            const DWORD bit = 1u << (letter - L'A');

            if ((drives & bit) == 0)
            {
                return std::wstring(1, letter);
            }
        }

        return std::nullopt;
    }

    std::optional<std::wstring> query_process_image_path(const HANDLE process)
    {
        std::wstring buffer(32768, L'\0');
        DWORD length = static_cast<DWORD>(buffer.size());

        if (!QueryFullProcessImageNameW(process, 0, buffer.data(), &length))
        {
            return std::nullopt;
        }

        buffer.resize(length);
        return buffer;
    }

    bool query_process_elevation(const HANDLE process)
    {
        HANDLE token = nullptr;

        if (!OpenProcessToken(process, TOKEN_QUERY, &token))
        {
            return false;
        }

        TOKEN_ELEVATION elevation = {};
        DWORD returned_size = 0;
        const bool elevated = GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &returned_size) && elevation.TokenIsElevated != 0;
        CloseHandle(token);
        return elevated;
    }

    bool buffer_contains_bytes(const std::uint8_t* const buffer, const std::size_t buffer_size, const std::uint8_t* const needle, const std::size_t needle_size)
    {
        if (buffer == nullptr || needle == nullptr || needle_size == 0 || buffer_size < needle_size)
        {
            return false;
        }

        const auto* const end = buffer + buffer_size;
        const auto* const it = std::search(buffer, end, needle, needle + needle_size);
        return it != end;
    }

    bool buffer_contains_ioc(const std::vector<std::uint8_t>& buffer, const std::string_view text)
    {
        if (text.empty())
        {
            return false;
        }

        const bool ascii_hit = buffer_contains_bytes(
            buffer.data(),
            buffer.size(),
            reinterpret_cast<const std::uint8_t*>(text.data()),
            text.size());

        if (ascii_hit)
        {
            return true;
        }

        std::vector<std::uint8_t> utf16le;
        utf16le.reserve(text.size() * sizeof(wchar_t));

        for (const char ch : text)
        {
            utf16le.push_back(static_cast<std::uint8_t>(ch));
            utf16le.push_back(0);
        }

        return buffer_contains_bytes(buffer.data(), buffer.size(), utf16le.data(), utf16le.size());
    }

    std::vector<const ioc_string_t*> collect_ioc_hits(const std::vector<std::uint8_t>& buffer, const std::array<ioc_string_t, 15>& iocs)
    {
        std::vector<const ioc_string_t*> hits;

        for (const auto& ioc : iocs)
        {
            if (buffer_contains_ioc(buffer, ioc.value))
            {
                hits.push_back(&ioc);
            }
        }

        return hits;
    }

    std::string join_labels(const std::vector<const ioc_string_t*>& hits)
    {
        std::ostringstream output;

        for (std::size_t i = 0; i < hits.size(); ++i)
        {
            if (i != 0)
            {
                output << ", ";
            }

            output << hits[i]->label;
        }

        return output.str();
    }

    int sum_scores(const std::vector<const ioc_string_t*>& hits)
    {
        int total = 0;

        for (const ioc_string_t* hit : hits)
        {
            total += hit->score;
        }

        return total;
    }

    int count_boot_capability_hits(const std::vector<const ioc_string_t*>& hits)
    {
        int total = 0;

        for (const ioc_string_t* hit : hits)
        {
            const std::string_view label(hit->label);

            if (label == "efi_boot_path" ||
                label == "boot_file_name" ||
                label == "bcdedit_setting" ||
                label == "mountvol_usage")
            {
                ++total;
            }
        }

        return total;
    }

    int count_pe_images_in_buffer(const std::vector<std::uint8_t>& buffer)
    {
        int count = 0;

        for (std::size_t i = 0; i + 0x40 < buffer.size(); ++i)
        {
            if (buffer[i] != 'M' || buffer[i + 1] != 'Z')
            {
                continue;
            }

            const std::uint32_t pe_offset = *reinterpret_cast<const std::uint32_t*>(&buffer[i + 0x3c]);

            if (pe_offset > 0x2000 || i + pe_offset + 4 > buffer.size())
            {
                continue;
            }

            if (buffer[i + pe_offset] == 'P' &&
                buffer[i + pe_offset + 1] == 'E' &&
                buffer[i + pe_offset + 2] == 0 &&
                buffer[i + pe_offset + 3] == 0)
            {
                ++count;
            }
        }

        return count;
    }

    std::optional<std::wstring> query_version_field(const std::filesystem::path& path, const wchar_t* field_name)
    {
        DWORD handle = 0;
        const DWORD version_size = GetFileVersionInfoSizeW(path.c_str(), &handle);

        if (version_size == 0)
        {
            return std::nullopt;
        }

        std::vector<std::uint8_t> version_buffer(version_size);

        if (!GetFileVersionInfoW(path.c_str(), handle, version_size, version_buffer.data()))
        {
            return std::nullopt;
        }

        struct lang_and_code_page_t
        {
            WORD language;
            WORD code_page;
        };

        lang_and_code_page_t* translations = nullptr;
        UINT translations_size = 0;

        if (!VerQueryValueW(version_buffer.data(), L"\\VarFileInfo\\Translation", reinterpret_cast<void**>(&translations), &translations_size) ||
            translations == nullptr ||
            translations_size < sizeof(lang_and_code_page_t))
        {
            return std::nullopt;
        }

        wchar_t query[128] = {};
        swprintf_s(query, L"\\StringFileInfo\\%04x%04x\\%s", translations[0].language, translations[0].code_page, field_name);

        wchar_t* value = nullptr;
        UINT value_size = 0;

        if (!VerQueryValueW(version_buffer.data(), query, reinterpret_cast<void**>(&value), &value_size) ||
            value == nullptr ||
            value_size == 0)
        {
            return std::nullopt;
        }

        return std::wstring(value, value_size - 1);
    }

    bool is_executable_extension(const std::filesystem::path& path)
    {
        const std::wstring extension = to_lower_copy(path.extension().wstring());
        return extension == L".efi" || extension == L".dll" || extension == L".exe";
    }

    bool is_standard_efi_name(const std::filesystem::path& path)
    {
        const std::wstring file_name = to_lower_copy(path.filename().wstring());

        return std::any_of(kStandardEfiExecutables.begin(), kStandardEfiExecutables.end(), [&](const std::wstring_view candidate)
        {
            return file_name == candidate;
        });
    }

    bool is_user_writable_path(const std::wstring& path)
    {
        const std::wstring lower = to_lower_copy(path);

        return lower.find(L"\\users\\") != std::wstring::npos ||
               lower.find(L"\\appdata\\") != std::wstring::npos ||
               lower.find(L"\\temp\\") != std::wstring::npos ||
               lower.find(L"\\downloads\\") != std::wstring::npos ||
               lower.find(L"\\desktop\\") != std::wstring::npos ||
               lower.find(L"\\documents\\") != std::wstring::npos;
    }

    std::vector<std::uint8_t> read_file_prefix(const std::filesystem::path& path, std::size_t max_bytes);

    void add_finding(std::vector<finding_t>& findings, std::set<std::string>& seen_keys, const std::string& category, const std::wstring& target, const std::string& ioc_label, const int score, const std::string& detail)
    {
        const std::string dedupe_key = category + "|" + ioc_label + "|" + narrow_utf8(target);

        if (!seen_keys.insert(dedupe_key).second)
        {
            return;
        }

        findings.push_back({category, target, ioc_label, score, detail});
    }

    void scan_ioc_strings_in_buffer(
        const std::vector<std::uint8_t>& buffer,
        const std::wstring& target,
        const std::string& category,
        std::vector<finding_t>& findings,
        std::set<std::string>& seen_keys,
        const std::array<ioc_string_t, 18>& iocs)
    {
        for (const auto& ioc : iocs)
        {
            if (buffer_contains_ioc(buffer, ioc.value))
            {
                add_finding(findings, seen_keys, category, target, ioc.label, ioc.score, ioc.value);
            }
        }
    }

    void scan_marker_strings_in_buffer(
        const std::vector<std::uint8_t>& buffer,
        const std::wstring& target,
        const std::string& category,
        std::vector<finding_t>& findings,
        std::set<std::string>& seen_keys)
    {
        for (const auto& ioc : kMarkerIocs)
        {
            if (buffer_contains_ioc(buffer, ioc.value))
            {
                add_finding(findings, seen_keys, category, target, ioc.label, ioc.score, ioc.value);
            }
        }
    }

    void scan_executable_file_heuristics(
        const std::filesystem::path& path,
        const std::wstring& target,
        const std::string& category,
        std::vector<finding_t>& findings,
        std::set<std::string>& seen_keys,
        const bool efi_context,
        const bool elevated_process_context)
    {
        if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
        {
            return;
        }

        const std::vector<std::uint8_t> bytes = read_file_prefix(path, 8ull * 1024ull * 1024ull);

        if (bytes.empty())
        {
            return;
        }

        const std::vector<const ioc_string_t*> capability_hits = collect_ioc_hits(bytes, kCapabilityIocs);

        if (capability_hits.size() >= 3)
        {
            const int cluster_score = std::min(45, sum_scores(capability_hits) + 8);
            add_finding(findings, seen_keys, category, target, "capability_cluster", cluster_score, join_labels(capability_hits));
        }

        const int embedded_pe_count = count_pe_images_in_buffer(bytes);

        if (embedded_pe_count >= 2)
        {
            const int score = embedded_pe_count >= 3 ? 35 : 22;
            add_finding(findings, seen_keys, category, target, "embedded_pe_payloads", score, "multiple PE images in a single executable");
        }

        const std::wstring company_name = query_version_field(path, L"CompanyName").value_or(L"");
        const std::wstring original_filename = query_version_field(path, L"OriginalFilename").value_or(L"");
        const std::wstring lower_company_name = to_lower_copy(company_name);
        const std::wstring lower_original_filename = to_lower_copy(original_filename);
        const std::wstring lower_filename = to_lower_copy(path.filename().wstring());
        const std::wstring lower_extension = to_lower_copy(path.extension().wstring());

        if (efi_context)
        {
            if (lower_extension == L".exe")
            {
                add_finding(findings, seen_keys, category, target, "efi_exe_anomaly", 70, "unexpected .exe inside EFI Microsoft Boot");
            }
            else if (lower_extension == L".dll")
            {
                add_finding(findings, seen_keys, category, target, "efi_dll_anomaly", 60, "unexpected .dll inside EFI Microsoft Boot");
            }
            else if (lower_extension == L".efi" && !is_standard_efi_name(path))
            {
                add_finding(findings, seen_keys, category, target, "efi_extra_executable", 30, "non-standard EFI executable name");
            }

            if (lower_filename == L"bootmgfw.efi")
            {
                if (lower_company_name.find(L"microsoft") == std::wstring::npos)
                {
                    add_finding(findings, seen_keys, category, target, "bootmgfw_company_mismatch", 65, "bootmgfw.efi does not identify as Microsoft");
                }

                if (!lower_original_filename.empty() && lower_original_filename.find(L"bootmgfw") == std::wstring::npos)
                {
                    add_finding(findings, seen_keys, category, target, "bootmgfw_name_mismatch", 45, "OriginalFilename does not match bootmgfw");
                }
            }
            else if (is_executable_extension(path) && lower_company_name.find(L"microsoft") == std::wstring::npos)
            {
                add_finding(findings, seen_keys, category, target, "efi_non_microsoft_binary", 25, "executable in EFI Microsoft Boot is not marked as Microsoft");
            }
        }

        if (elevated_process_context && is_user_writable_path(path.wstring()) && capability_hits.size() >= 3)
        {
            add_finding(findings, seen_keys, category, target, "elevated_user_path_binary", 25, "elevated process from user-writable path with boot or introspection capability cluster");
        }

        scan_ioc_strings_in_buffer(bytes, target, category, findings, seen_keys, kProcessIocs);
        scan_marker_strings_in_buffer(bytes, target, category, findings, seen_keys);
    }

    std::vector<std::uint8_t> read_file_prefix(const std::filesystem::path& path, const std::size_t max_bytes)
    {
        HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (file == INVALID_HANDLE_VALUE)
        {
            return {};
        }

        LARGE_INTEGER file_size = {};
        GetFileSizeEx(file, &file_size);

        const std::size_t bytes_to_read = static_cast<std::size_t>(std::min<LONGLONG>(file_size.QuadPart, static_cast<LONGLONG>(max_bytes)));
        std::vector<std::uint8_t> buffer(bytes_to_read);

        DWORD bytes_read = 0;
        if (bytes_to_read != 0)
        {
            ReadFile(file, buffer.data(), static_cast<DWORD>(bytes_to_read), &bytes_read, nullptr);
        }

        CloseHandle(file);
        buffer.resize(bytes_read);
        return buffer;
    }

    std::optional<efi_file_ioc_t> match_b_drive_ioc(const std::filesystem::path& path)
    {
        const std::wstring lower_filename = to_lower_copy(path.filename().wstring());

        for (const auto& ioc : kEfiFileIocs)
        {
            const std::wstring ioc_filename = to_lower_copy(std::filesystem::path(ioc.relative_path).filename().wstring());

            if (lower_filename == ioc_filename)
            {
                return ioc;
            }
        }

        return std::nullopt;
    }

    void scan_b_drive_executable_heuristics(
        const std::vector<std::uint8_t>& bytes,
        const std::wstring& target,
        std::vector<finding_t>& findings,
        std::set<std::string>& seen_keys)
    {
        const std::vector<const ioc_string_t*> capability_hits = collect_ioc_hits(bytes, kCapabilityIocs);

        if (capability_hits.size() >= 3 && count_boot_capability_hits(capability_hits) >= 2)
        {
            const int cluster_score = std::min(45, sum_scores(capability_hits) + 8);
            add_finding(findings, seen_keys, "b_drive_heuristic", target, "capability_cluster", cluster_score, join_labels(capability_hits));
        }

        const int embedded_pe_count = count_pe_images_in_buffer(bytes);

        if (embedded_pe_count >= 2)
        {
            const int score = embedded_pe_count >= 3 ? 35 : 22;
            add_finding(findings, seen_keys, "b_drive_heuristic", target, "embedded_pe_payloads", score, "multiple PE images in a single executable");
        }
    }

    void scan_process_memory(HANDLE process, process_report_t& report)
    {
        std::set<std::string> seen_keys;
        std::uintptr_t address = 0;

        while (report.scanned_bytes < kMaxProcessScanBytes)
        {
            MEMORY_BASIC_INFORMATION mbi = {};

            if (VirtualQueryEx(process, reinterpret_cast<const void*>(address), &mbi, sizeof(mbi)) != sizeof(mbi))
            {
                break;
            }

            const std::uintptr_t next_address = reinterpret_cast<std::uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;

            if (mbi.State == MEM_COMMIT && is_process_readable(mbi.Protect) && (mbi.Type == MEM_IMAGE || mbi.Type == MEM_PRIVATE))
            {
                const std::size_t region_cap = (mbi.Type == MEM_IMAGE) ? kMaxImageRegionScanBytes : kMaxPrivateRegionScanBytes;
                std::size_t bytes_to_scan = static_cast<std::size_t>(std::min<std::uintptr_t>(mbi.RegionSize, region_cap));
                bytes_to_scan = std::min(bytes_to_scan, kMaxProcessScanBytes - report.scanned_bytes);

                if (bytes_to_scan != 0)
                {
                    std::vector<std::uint8_t> buffer(bytes_to_scan);
                    SIZE_T bytes_read = 0;

                    if (ReadProcessMemory(process, mbi.BaseAddress, buffer.data(), bytes_to_scan, &bytes_read) && bytes_read != 0)
                    {
                        buffer.resize(static_cast<std::size_t>(bytes_read));
                        const std::wstring target = report.name + L" (pid " + std::to_wstring(report.pid) + L")";

                        scan_ioc_strings_in_buffer(buffer, target, "process_memory", report.findings, seen_keys, kProcessIocs);
                        scan_marker_strings_in_buffer(buffer, target, "process_memory", report.findings, seen_keys);
                        report.scanned_bytes += static_cast<std::size_t>(bytes_read);
                    }
                }
            }

            if (next_address <= address)
            {
                break;
            }

            address = next_address;
        }

    }

    process_report_t scan_single_process(const PROCESSENTRY32W& entry)
    {
        process_report_t report = {};
        report.pid = entry.th32ProcessID;
        report.name = entry.szExeFile;

        const DWORD current_pid = GetCurrentProcessId();

        if (report.pid == 0 || report.pid == current_pid)
        {
            return report;
        }

        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, report.pid);

        if (process == nullptr)
        {
            return report;
        }

        const std::optional<std::wstring> image_path = query_process_image_path(process);
        if (image_path)
        {
            report.path = *image_path;
        }

        report.elevated = query_process_elevation(process);

        scan_process_memory(process, report);

        if (!report.path.empty())
        {
            std::set<std::string> seen_keys;
            scan_executable_file_heuristics(report.path, report.name + L" (pid " + std::to_wstring(report.pid) + L")", "process_image", report.findings, seen_keys, false, report.elevated);
        }

        report.score = 0;

        for (const finding_t& finding : report.findings)
        {
            report.score += finding.score;
        }

        CloseHandle(process);
        return report;
    }

    void scan_processes(system_report_t& report)
    {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (snapshot == INVALID_HANDLE_VALUE)
        {
            return;
        }

        PROCESSENTRY32W entry = {};
        entry.dwSize = sizeof(entry);

        if (!Process32FirstW(snapshot, &entry))
        {
            CloseHandle(snapshot);
            return;
        }

        do
        {
            process_report_t process_report = scan_single_process(entry);

            if (!process_report.findings.empty())
            {
                report.total_score += process_report.score;
                report.process_reports.push_back(std::move(process_report));
            }
        }
        while (Process32NextW(snapshot, &entry));

        CloseHandle(snapshot);
    }

    void scan_efi(system_report_t& report)
    {
        report.efi_scan_attempted = true;

        if (!report.is_admin)
        {
            return;
        }

        const std::optional<std::wstring> drive_letter = choose_free_drive_letter();

        if (!drive_letter)
        {
            return;
        }

        const std::wstring mount_root = *drive_letter + L":";

        if (!run_hidden_command(L"mountvol " + mount_root + L" /S"))
        {
            return;
        }

        report.efi_scan_succeeded = true;
        std::set<std::string> seen_keys;

        const std::filesystem::path root = mount_root + L"\\";

        for (const auto& ioc : kEfiFileIocs)
        {
            const std::filesystem::path target_path = root / ioc.relative_path;

            if (!std::filesystem::exists(target_path))
            {
                continue;
            }

            add_finding(report.efi_findings, seen_keys, "efi_file", target_path.wstring(), ioc.label, ioc.score, "present on EFI partition");
            report.total_score += ioc.score;

            const std::vector<std::uint8_t> file_bytes = read_file_prefix(target_path, 2ull * 1024ull * 1024ull);

            if (!file_bytes.empty())
            {
                scan_ioc_strings_in_buffer(file_bytes, target_path.wstring(), "efi_file_content", report.efi_findings, seen_keys, kProcessIocs);
                scan_marker_strings_in_buffer(file_bytes, target_path.wstring(), "efi_file_content", report.efi_findings, seen_keys);
            }
        }

        for (const finding_t& finding : report.efi_findings)
        {
            if (finding.category == "efi_file_content")
            {
                report.total_score += finding.score;
            }
        }

        std::set<std::string> heuristic_seen_keys;

        const std::filesystem::path boot_root = root / L"EFI\\Microsoft\\Boot";

        if (std::filesystem::exists(boot_root))
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(boot_root, std::filesystem::directory_options::skip_permission_denied))
            {
                if (!entry.is_regular_file() || !is_executable_extension(entry.path()))
                {
                    continue;
                }

                scan_executable_file_heuristics(entry.path(), entry.path().wstring(), "efi_heuristic", report.efi_findings, heuristic_seen_keys, true, false);
            }
        }

        for (const finding_t& finding : report.efi_findings)
        {
            if (finding.category == "efi_heuristic")
            {
                report.total_score += finding.score;
            }
        }

        run_hidden_command(L"mountvol " + mount_root + L" /D");
    }

    void scan_b_drive(system_report_t& report)
    {
        report.b_drive_scan_attempted = true;

        const std::filesystem::path root = L"B:\\";

        if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root))
        {
            return;
        }

        report.b_drive_scan_succeeded = true;
        std::set<std::string> seen_keys;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(root, std::filesystem::directory_options::skip_permission_denied))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            const std::filesystem::path path = entry.path();
            const std::optional<efi_file_ioc_t> matched_ioc = match_b_drive_ioc(path);
            const bool should_scan_file = matched_ioc.has_value() || is_executable_extension(path);

            if (!should_scan_file)
            {
                continue;
            }

            if (matched_ioc)
            {
                const int file_score = std::max(20, matched_ioc->score - 20);
                add_finding(report.b_drive_findings, seen_keys, "b_drive_file", path.wstring(), matched_ioc->label, file_score, "present on B drive");
                report.total_score += file_score;
            }

            const std::vector<std::uint8_t> bytes = read_file_prefix(path, 4ull * 1024ull * 1024ull);

            if (bytes.empty())
            {
                continue;
            }

            if (is_executable_extension(path))
            {
                scan_b_drive_executable_heuristics(bytes, path.wstring(), report.b_drive_findings, seen_keys);
            }

            scan_ioc_strings_in_buffer(bytes, path.wstring(), "b_drive_file_content", report.b_drive_findings, seen_keys, kProcessIocs);
            scan_marker_strings_in_buffer(bytes, path.wstring(), "b_drive_file_content", report.b_drive_findings, seen_keys);
        }

        for (const finding_t& finding : report.b_drive_findings)
        {
            if (finding.category == "b_drive_heuristic" || finding.category == "b_drive_file_content")
            {
                report.total_score += finding.score;
            }
        }
    }

    std::string verdict_from_report(const system_report_t& report)
    {
        const bool has_strong_efi_finding = std::any_of(report.efi_findings.begin(), report.efi_findings.end(), [](const finding_t& finding)
        {
            return finding.category == "efi_file" && finding.score >= 60;
        });

        const bool has_high_process_score = std::any_of(report.process_reports.begin(), report.process_reports.end(), [](const process_report_t& process)
        {
            return process.score >= 80;
        });

        if (has_strong_efi_finding || has_high_process_score || report.total_score >= 100)
        {
            return "HIGH_CONFIDENCE_DETECTION";
        }

        if (report.total_score >= 35)
        {
            return "REVIEW";
        }

        return "LOW_SIGNAL";
    }

    std::optional<std::filesystem::path> query_module_directory()
    {
        std::wstring buffer(MAX_PATH, L'\0');

        for (;;)
        {
            const DWORD copied = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));

            if (copied == 0)
            {
                return std::nullopt;
            }

            if (copied < buffer.size() - 1)
            {
                buffer.resize(copied);
                return std::filesystem::path(buffer).parent_path();
            }

            buffer.resize(buffer.size() * 2);
        }
    }

    std::filesystem::path build_report_path()
    {
        SYSTEMTIME local_time = {};
        GetLocalTime(&local_time);

        std::ostringstream file_name;
        file_name
            << std::setfill('0')
            << std::setw(4) << local_time.wYear
            << std::setw(2) << local_time.wMonth
            << std::setw(2) << local_time.wDay
            << "-"
            << std::setw(2) << local_time.wHour
            << std::setw(2) << local_time.wMinute
            << std::setw(2) << local_time.wSecond
            << ".txt";

        const std::filesystem::path base_directory = query_module_directory().value_or(std::filesystem::current_path());
        const std::filesystem::path report_directory = base_directory / "reports";
        std::filesystem::create_directories(report_directory);
        return report_directory / file_name.str();
    }

    void print_findings(std::ostream& output, const std::vector<finding_t>& findings)
    {
        for (const finding_t& finding : findings)
        {
            output
                << "  [" << finding.category << "] "
                << narrow_utf8(finding.target)
                << " -> "
                << finding.ioc_label
                << " (+" << finding.score << ")"
                << " : "
                << finding.detail
                << "\n";
        }
    }

    std::string build_report_text(const system_report_t& report)
    {
        std::ostringstream output;

        output << "hyper-reV detector\n";
        output << "mode: fast IOC scan for EFI artifacts, B drive artifacts, and process-memory strings\n\n";

        if (!report.is_admin)
        {
            output << "warning: not running as administrator, EFI scan will be skipped and some processes may be unreadable.\n\n";
        }

        output << "verdict: " << verdict_from_report(report) << "\n";
        output << "total_score: " << report.total_score << "\n";
        output << "efi_scan: " << (report.efi_scan_succeeded ? "ok" : (report.efi_scan_attempted ? "skipped_or_failed" : "not_attempted")) << "\n";
        output << "b_drive_scan: " << (report.b_drive_scan_succeeded ? "ok" : (report.b_drive_scan_attempted ? "not_found_or_failed" : "not_attempted")) << "\n";
        output << "process_hits: " << report.process_reports.size() << "\n\n";

        output << "runtime_hardening:\n";
        output << "  heap_termination: " << (report.hardening.heap_termination_enabled ? "ok" : "failed_or_skipped") << "\n";
        output << "  dll_search_order: " << (report.hardening.dll_search_order_hardened ? "ok" : "failed_or_skipped") << "\n";
        output << "  extension_points: " << (report.hardening.extension_points_disabled ? "ok" : "failed_or_skipped") << "\n";
        output << "  image_load_policy: " << (report.hardening.image_load_policy_applied ? "ok" : "failed_or_skipped") << "\n";
        output << "  dynamic_code_policy: " << (report.hardening.dynamic_code_policy_applied ? "ok" : "failed_or_skipped") << "\n";
        output << "  debugger_present: " << (report.debugger_present ? "yes" : "no") << "\n";
        output << "  last_error: " << report.hardening.last_error << "\n\n";

        if (!report.efi_findings.empty())
        {
            output << "efi_findings:\n";
            print_findings(output, report.efi_findings);
            output << "\n";
        }
        else
        {
            output << "efi_findings: none\n\n";
        }

        if (!report.b_drive_findings.empty())
        {
            output << "b_drive_findings:\n";
            print_findings(output, report.b_drive_findings);
            output << "\n";
        }
        else
        {
            output << "b_drive_findings: none\n\n";
        }

        if (!report.process_reports.empty())
        {
            output << "process_findings:\n";

            for (const process_report_t& process : report.process_reports)
            {
                output
                    << " process "
                    << narrow_utf8(process.name)
                    << " (pid "
                    << process.pid
                    << ")"
                    << " score="
                    << process.score
                    << " scanned_bytes="
                    << process.scanned_bytes
                    << "\n";

                print_findings(output, process.findings);
            }
        }
        else
        {
            output << "process_findings: none\n";
        }

        return output.str();
    }
}

int wmain()
{
    system_report_t report = {};
    report.hardening = protec::apply_baseline_hardening();
    report.debugger_present = protec::is_debugger_likely_present();
    report.is_admin = is_running_as_admin();

    scan_efi(report);
    scan_b_drive(report);
    scan_processes(report);

    std::sort(report.process_reports.begin(), report.process_reports.end(), [](const process_report_t& left, const process_report_t& right)
    {
        return left.score > right.score;
    });

    const std::string report_text = build_report_text(report);
    std::cout << report_text;

    try
    {
        const std::filesystem::path report_path = build_report_path();
        std::ofstream report_file(report_path, std::ios::binary);

        if (report_file)
        {
            report_file << report_text;
            report_file.close();
            std::cout << "\nreport_file: " << report_path.string() << "\n";
        }
        else
        {
            std::cout << "\nreport_file: write_failed\n";
        }
    }
    catch (const std::exception&)
    {
        std::cout << "\nreport_file: write_failed\n";
    }

    return 0;
}
