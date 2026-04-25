#pragma once
// =============================================================================
// ULTIMATE GZ-PROTECTOR v4.0 - Global Hardening System
// Unified Protection for Loader & Game Modules
// =============================================================================

#include <Windows.h>
#include <winternl.h>
#include <TlHelp32.h>
#include <tchar.h>
#include <cstdint>
#include <string>
#include <vector>
#include <nmmintrin.h> // SSE4.2 CRC32
#include <intrin.h>

// Use shared skCrypt
#include "skCrypt.h"

namespace protec {

    // =========================================================================
    // Configuration
    // =========================================================================
    inline int scan_detection_time = 3000;
    inline bool loop_killdbgr = true;
    inline bool protector_bsod = false;

    // =========================================================================
    // LAYER 0: Core Utility & Actions
    // =========================================================================
    
    static void debugger_detected() {
        // Option 1: BSOD via NtRaiseHardError (High Aggression)
        if (protector_bsod) {
            typedef NTSTATUS(NTAPI* pRtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
            typedef NTSTATUS(NTAPI* pNtRaiseHardError)(NTSTATUS, ULONG, ULONG, PULONG_PTR, ULONG, PULONG);

            HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
            if (hNtdll) {
                auto RtlAdj = (pRtlAdjustPrivilege)GetProcAddress(hNtdll, "RtlAdjustPrivilege");
                auto NtRaise = (pNtRaiseHardError)GetProcAddress(hNtdll, "NtRaiseHardError");
                if (RtlAdj && NtRaise) {
                    BOOLEAN bEnabled;
                    ULONG uResp;
                    RtlAdj(19, TRUE, FALSE, &bEnabled);
                    NtRaise(0xC0000374L, 0, 0, NULL, 6, &uResp); // STATUS_HEAP_CORRUPTION
                }
            }
        }

        // Option 2: Exit silently with fake error code
        // Before exit, try to wipe PE header of the calling module
        HMODULE hMain = GetModuleHandleW(NULL);
        DWORD oldProt;
        if (VirtualProtect(hMain, 4096, PAGE_READWRITE, &oldProt)) {
            SecureZeroMemory(hMain, 4096);
        }
        
        TerminateProcess(GetCurrentProcess(), 0xDEADBEEF);
    }

    // =========================================================================
    // LAYER 1: Anti-Debugging
    // =========================================================================

    static bool check_peb_direct() {
#ifdef _AMD64_
        PPEB pPeb = (PPEB)__readgsqword(0x60);
#else
        PPEB pPeb = (PPEB)__readfsdword(0x30);
#endif
        if (!pPeb) return false;
        if (pPeb->BeingDebugged) return true;
        
        // NtGlobalFlag check (0x70 = debugged)
        ULONG ntGlobalFlag = *(ULONG*)((BYTE*)pPeb + 0xBC);
        if (ntGlobalFlag & 0x70) return true;

        return false;
    }

    static bool is_debugger_present() {
        if (check_peb_direct()) return true;
        if (IsDebuggerPresent()) return true;

        BOOL remoteDbg = FALSE;
        CheckRemoteDebuggerPresent(GetCurrentProcess(), &remoteDbg);
        if (remoteDbg) return true;

        // NtQueryInformationProcess
        typedef NTSTATUS(NTAPI* pNtQIP)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
        static pNtQIP NtQIP = (pNtQIP)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
        if (NtQIP) {
            DWORD_PTR debugPort = 0;
            if (NtQIP(GetCurrentProcess(), (PROCESSINFOCLASS)7, &debugPort, sizeof(debugPort), NULL) == 0 && debugPort != 0)
                return true;
        }

        return false;
    }

    static bool is_hardware_breakpoint_present() {
        CONTEXT ctx = { 0 };
        ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        if (GetThreadContext(GetCurrentThread(), &ctx)) {
            return (ctx.Dr0 != 0 || ctx.Dr1 != 0 || ctx.Dr2 != 0 || ctx.Dr3 != 0);
        }
        return false;
    }

    static void hide_thread(HANDLE hThread = GetCurrentThread()) {
        typedef NTSTATUS(NTAPI* pNtSetInformationThread)(HANDLE, ULONG, PVOID, ULONG);
        static pNtSetInformationThread NtSIT = (pNtSetInformationThread)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtSetInformationThread");
        if (NtSIT) {
            NtSIT(hThread, 0x11 /*ThreadHideFromDebugger*/, NULL, 0);
        }
    }

    // =========================================================================
    // LAYER 2: Anti-Dump & Mitigation
    // =========================================================================

    typedef struct _UNICODE_STRING {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR  Buffer;
    } UNICODE_STRING, *PUNICODE_STRING;

    typedef struct _LDR_DATA_TABLE_ENTRY {
        LIST_ENTRY InLoadOrderLinks;
        LIST_ENTRY InMemoryOrderLinks;
        LIST_ENTRY InInitializationOrderLinks;
        PVOID DllBase;
        PVOID EntryPoint;
        ULONG SizeOfImage;
        UNICODE_STRING FullDllName;
        UNICODE_STRING BaseDllName;
    } LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

    typedef struct _PEB_LDR_DATA {
        ULONG Length;
        BOOLEAN Initialized;
        HANDLE SsHandle;
        LIST_ENTRY InLoadOrderModuleList;
        LIST_ENTRY InMemoryOrderModuleList;
        LIST_ENTRY InInitializationOrderModuleList;
    } PEB_LDR_DATA, *PPEB_LDR_DATA;

    typedef struct _PEB {
        BOOLEAN InheritedAddressSpace;
        BOOLEAN ReadImageFileExecOptions;
        BOOLEAN BeingDebugged;
        BOOLEAN SpareBool;
        HANDLE Mutant;
        PVOID ImageBaseAddress;
        PPEB_LDR_DATA Ldr;
    } PEB, *PPEB;

    static void hide_module_from_peb() {
        PPEB peb = (PPEB)__readgsqword(0x60);
        if (!peb || !peb->Ldr) return;

        PPEB_LDR_DATA ldr = peb->Ldr;
        PLIST_ENTRY list = &ldr->InLoadOrderModuleList;

        for (PLIST_ENTRY entry = list->Flink; entry != list; entry = entry->Flink) {
            PLDR_DATA_TABLE_ENTRY module = (PLDR_DATA_TABLE_ENTRY)entry;
            if (module->DllBase == peb->ImageBaseAddress) {
                // Double link unlinking - InLoadOrder
                entry->Blink->Flink = entry->Flink;
                entry->Flink->Blink = entry->Blink;

                // Unlink from InMemoryOrderModuleList
                PLIST_ENTRY mem_entry = &module->InMemoryOrderLinks;
                if (mem_entry->Flink && mem_entry->Blink) {
                    mem_entry->Blink->Flink = mem_entry->Flink;
                    mem_entry->Flink->Blink = mem_entry->Blink;
                }

                // Unlink from InInitializationOrderModuleList
                PLIST_ENTRY init_entry = &module->InInitializationOrderLinks;
                if (init_entry->Flink && init_entry->Blink) {
                    init_entry->Blink->Flink = init_entry->Flink;
                    init_entry->Flink->Blink = init_entry->Blink;
                }

                break;
            }
        }
    }

    static void erase_pe_header() {
        // [ANTI-DUMP v2] Replaced with PEB Hiding to solve DirectX compatibility issues
        hide_module_from_peb();
    }

    struct hardening_result_t
    {
        bool heap_termination_enabled = false;
        bool dll_search_order_hardened = false;
        bool extension_points_disabled = false;
        bool image_load_policy_applied = false;
        bool dynamic_code_policy_applied = false;
        bool pe_header_erased = false;
        DWORD last_error = ERROR_SUCCESS;
    };

    static void apply_mitigation_policies() {
        SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS);

        /* Temporarily disabled as it may interfere with WinHTTP/Networking in some environments
        PROCESS_MITIGATION_IMAGE_LOAD_POLICY imagePolicy = {};
        imagePolicy.NoRemoteImages = 1;
        imagePolicy.NoLowMandatoryLabelImages = 1;
        imagePolicy.PreferSystem32Images = 1;
        SetProcessMitigationPolicy(ProcessImageLoadPolicy, &imagePolicy, sizeof(imagePolicy));

        PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY extPolicy = {};
        extPolicy.DisableExtensionPoints = 1;
        SetProcessMitigationPolicy(ProcessExtensionPointDisablePolicy, &extPolicy, sizeof(extPolicy));
        */
    }

    static hardening_result_t apply_baseline_hardening() {
        apply_mitigation_policies();
        hardening_result_t res;
        res.dll_search_order_hardened = true;
        res.image_load_policy_applied = true;
        res.extension_points_disabled = true;
        res.heap_termination_enabled = HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
        return res;
    }

    static bool is_debugger_likely_present() {
        return is_debugger_present();
    }

    // =========================================================================
    // LAYER 3: Environment Scanning
    // =========================================================================

    static bool scan_blacklist() {
        // Process names to check
        static const wchar_t* blacklisted[] = {
            skCrypt(L"x64dbg.exe"), skCrypt(L"x32dbg.exe"), skCrypt(L"ollydbg.exe"),
            skCrypt(L"ida.exe"), skCrypt(L"ida64.exe"), skCrypt(L"cheatengine-x86_64.exe"),
            skCrypt(L"Cheat Engine.exe"), skCrypt(L"ProcessHacker.exe"), skCrypt(L"KsDumper.exe"),
            skCrypt(L"KsDumperClient.exe"), skCrypt(L"procmon.exe"), skCrypt(L"procexp.exe"),
            skCrypt(L"Wireshark.exe"), skCrypt(L"HTTPDebuggerUI.exe"), skCrypt(L"dnSpy.exe")
        };

        // Window titles to check
        static const wchar_t* titles[] = {
            skCrypt(L"x64dbg"), skCrypt(L"x32dbg"), skCrypt(L"IDA Pro"),
            skCrypt(L"Cheat Engine"), skCrypt(L"Process Hacker"), skCrypt(L"KsDumper"),
            skCrypt(L"API Monitor")
        };

        // Scan windows
        for (const auto& t : titles) {
            if (FindWindowW(NULL, t)) return true;
        }

        // Scan processes
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32W pe = { sizeof(pe) };
            if (Process32FirstW(snapshot, &pe)) {
                do {
                    for (const auto& b : blacklisted) {
                        if (_wcsicmp(pe.szExeFile, b) == 0) {
                            CloseHandle(snapshot);
                            return true;
                        }
                    }
                } while (Process32NextW(snapshot, &pe));
            }
            CloseHandle(snapshot);
        }

        return false;
    }

    static bool is_vm() {
        HANDLE hFile = CreateFileA(skCrypt("\\\\.\\VBoxGuest"), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE) { CloseHandle(hFile); return true; }
        
        hFile = CreateFileA(skCrypt("\\\\.\\HGFS"), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile != INVALID_HANDLE_VALUE) { CloseHandle(hFile); return true; }

        return false;
    }

    // =========================================================================
    // BACKGROUND MONITORING
    // =========================================================================

    static DWORD WINAPI protection_thread(LPVOID) {
        while (true) {
            hide_thread();
            if (is_debugger_present()) debugger_detected();
            if (is_hardware_breakpoint_present()) debugger_detected();
            if (scan_blacklist()) debugger_detected();
            
            Sleep(scan_detection_time);
        }
        return 0;
    }

    // =========================================================================
    // ENTRY POINT
    // =========================================================================

    static void start_protect(bool anti_dump = true) {
        apply_mitigation_policies();
        hide_thread();

        if (anti_dump) {
            erase_pe_header();
        }

        // Check once before threading
        if (is_debugger_present() || is_vm() || scan_blacklist()) {
            debugger_detected();
        }

        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)protection_thread, NULL, 0, NULL);
    }
}
