#pragma once
// =============================================================================
// GZ-PROTECTOR v3.0 - Ultimate Anti-Debug / Anti-Crack / Anti-Dump System
// Combines techniques from: Anti-Debugger-Protector-Loader, anti-crack-system-ReFo,
// AntiDebugging (TextSectionHasher), Anti-CrackMe-V2
// Fully Unicode (WCHAR) compatible - No VMProtect SDK dependency
// =============================================================================

#include <Windows.h>
#include <winternl.h>
#include <TlHelp32.h>
#include <tchar.h>
#include <cstdint>
#include <string>
#include <nmmintrin.h> // SSE4.2 CRC32
#include <cstdlib>

// Use the project's existing skCrypt for compile-time string encryption
#include "../skCrypt.h"

namespace protector {

    // =========================================================================
    // Configuration Variables
    // =========================================================================
    int scan_detection_time = 1000;
    bool scan_exe = true;
    bool scan_title = true;
    bool scan_driver = true;
    bool loop_killdbgr = false;
    bool debug_log = false;
    bool protector_bsod = false;

    // =========================================================================
    // LAYER 0: Core Utility - Find Process by Name (Wide-char)
    // =========================================================================
    static DWORD find_process_w(const wchar_t* procName) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return 0;

        PROCESSENTRY32W pe = { sizeof(PROCESSENTRY32W) };
        if (Process32FirstW(snapshot, &pe)) {
            do {
                if (_wcsicmp(procName, pe.szExeFile) == 0) {
                    CloseHandle(snapshot);
                    return pe.th32ProcessID;
                }
            } while (Process32NextW(snapshot, &pe));
        }
        CloseHandle(snapshot);
        return 0;
    }

    // =========================================================================
    // LAYER 0: Detected Action - What to do when a debugger/cracker is found
    // =========================================================================
    static void debugger_detected() {
        // Option 1: BSOD via NtRaiseHardError
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

        // Option 2: Silent crash / exit (always as fallback)
        // Corrupt own memory to prevent analysis
        char* base = (char*)GetModuleHandleW(NULL);
        DWORD oldProt;
        if (VirtualProtect(base, 4096, PAGE_READWRITE, &oldProt)) {
            ZeroMemory(base, 4096); // Wipe PE header
        }
        TerminateProcess(GetCurrentProcess(), 0xDEAD);
    }

    // =========================================================================
    // LAYER 1: Anti-Debug (API-based + PEB direct access)
    //   - IsDebuggerPresent API
    //   - PEB->BeingDebugged direct read (bypass API hooks)
    //   - PEB->NtGlobalFlag heap debug flags (from Anti-CrackMe-V2)
    //   - PEB->CrossProcessFlags VEH detection (from VEH_Checker)
    //   - CheckRemoteDebuggerPresent
    //   - NtQueryInformationProcess (DebugPort, DebugObjectHandle, DebugFlags)
    //   - SystemKernelDebuggerInformation (from Anti-Debugger-Protector)
    //   - OutputDebugString trick
    //   - Hardware Breakpoint detection (DR registers)
    // =========================================================================

    // [Anti-CrackMe-V2] PEB heap debug flags set by debugger
    #define FLG_HEAP_ENABLE_TAIL_CHECK   0x10
    #define FLG_HEAP_ENABLE_FREE_CHECK   0x20
    #define FLG_HEAP_VALIDATE_PARAMETERS 0x40
    #define STATUS_GLOBAL_FLAG_DEBUGGED (FLG_HEAP_ENABLE_TAIL_CHECK | FLG_HEAP_ENABLE_FREE_CHECK | FLG_HEAP_VALIDATE_PARAMETERS)

    static bool check_peb_direct() {
        // Direct PEB access - cannot be hooked by user-mode hooks
#ifdef _AMD64_
        PPEB pPeb = (PPEB)__readgsqword(0x60);
#else
        PPEB pPeb = (PPEB)__readfsdword(0x30);
#endif
        if (!pPeb) return false;

        // [Anti-CrackMe-V2] Check BeingDebugged directly from PEB
        if (pPeb->BeingDebugged) return true;

        // [Anti-CrackMe-V2] Check NtGlobalFlag - debugger sets heap flags 0x70
        // NtGlobalFlag is at offset 0xBC (x86) / 0xBC (x64) in PEB
        // Using Reserved4 to access it safely
        ULONG ntGlobalFlag = *(ULONG*)((BYTE*)pPeb + 0xBC);
        if (ntGlobalFlag & STATUS_GLOBAL_FLAG_DEBUGGED) return true;

        // NOTE: CrossProcessFlags VEH check removed - causes false positives
        // when legitimate DLLs (ImGui, DirectX) install VEH handlers

        return false;
    }

    static bool check_api_debugger() {
        // 1. Direct PEB checks (unhookable - from Anti-CrackMe-V2 + VEH_Checker)
        if (check_peb_direct()) return true;

        // 2. IsDebuggerPresent (API version - may be hooked)
        if (IsDebuggerPresent()) return true;

        // 3. CheckRemoteDebuggerPresent
        BOOL remoteDbg = FALSE;
        CheckRemoteDebuggerPresent(GetCurrentProcess(), &remoteDbg);
        if (remoteDbg) return true;

        // 4. NtQueryInformationProcess - Multiple checks
        typedef NTSTATUS(NTAPI* pNtQIP)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
        HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
        if (hNtdll) {
            auto NtQIP = (pNtQIP)GetProcAddress(hNtdll, "NtQueryInformationProcess");
            if (NtQIP) {
                // 4a. ProcessDebugPort (class 7)
                DWORD_PTR debugPort = 0;
                if (NtQIP(GetCurrentProcess(), (PROCESSINFOCLASS)7, &debugPort, sizeof(debugPort), NULL) == 0 && debugPort != 0)
                    return true;

                // 4b. ProcessDebugObjectHandle (class 0x1E)
                HANDLE debugObj = NULL;
                if (NtQIP(GetCurrentProcess(), (PROCESSINFOCLASS)0x1E, &debugObj, sizeof(debugObj), NULL) == 0 && debugObj != NULL)
                    return true;

                // 4c. ProcessDebugFlags (class 0x1F)
                DWORD debugFlags = 1;
                if (NtQIP(GetCurrentProcess(), (PROCESSINFOCLASS)0x1F, &debugFlags, sizeof(debugFlags), NULL) == 0 && debugFlags == 0)
                    return true;
            }

            // 5. [Anti-Debugger-Protector] SystemKernelDebuggerInformation
            typedef struct _SYSTEM_KERNEL_DEBUGGER_INFO {
                BOOLEAN DebuggerEnabled;
                BOOLEAN DebuggerNotPresent;
            } SYSTEM_KERNEL_DEBUGGER_INFO;
            typedef NTSTATUS(NTAPI* pZwQuerySystemInformation)(ULONG, PVOID, ULONG, PULONG);
            auto ZwQSI = (pZwQuerySystemInformation)GetProcAddress(hNtdll, "ZwQuerySystemInformation");
            if (ZwQSI) {
                SYSTEM_KERNEL_DEBUGGER_INFO kdbgInfo = {};
                // SystemKernelDebuggerInformation = 35
                if (ZwQSI(35, &kdbgInfo, sizeof(kdbgInfo), NULL) == 0) {
                    if (kdbgInfo.DebuggerEnabled && !kdbgInfo.DebuggerNotPresent)
                        return true;
                }
            }
        }

        // 6. OutputDebugString trick
        SetLastError(0);
        OutputDebugStringA("GZ_Security_Probe");
        if (GetLastError() != 0)
            return true;

        return false;
    }

    static bool check_hardware_breakpoints() {
        CONTEXT ctx = { 0 };
        ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        HANDLE hThread = GetCurrentThread();
        if (GetThreadContext(hThread, &ctx)) {
            if (ctx.Dr0 != 0 || ctx.Dr1 != 0 || ctx.Dr2 != 0 || ctx.Dr3 != 0 ||
                ctx.Dr6 != 0 || ctx.Dr7 != 0) {
                return true;
            }
        }
        return false;
    }

    static void hide_thread_from_debugger() {
        typedef NTSTATUS(NTAPI* pNtSetInformationThread)(HANDLE, ULONG, PVOID, ULONG);
        HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
        if (hNtdll) {
            auto NtSIT = (pNtSetInformationThread)GetProcAddress(hNtdll, "NtSetInformationThread");
            if (NtSIT) {
                NtSIT(GetCurrentThread(), 0x11 /*ThreadHideFromDebugger*/, NULL, 0);
            }
        }
    }

    // =========================================================================
    // LAYER 2: Anti-Dump
    //   - Erase PE Header from memory
    //   - Corrupt SizeOfImage in PEB->Ldr to break dump tools
    // =========================================================================
    static void erase_pe_header() {
        DWORD oldProtect = 0;
        char* baseAddress = (char*)GetModuleHandleW(NULL);
        if (VirtualProtect(baseAddress, 4096, PAGE_READWRITE, &oldProtect)) {
            ZeroMemory(baseAddress, 4096);
        }
    }

    static void corrupt_size_of_image() {
#ifdef _AMD64_
        const auto peb = (PPEB)__readgsqword(0x60);
#else
        const auto peb = (PPEB)__readfsdword(0x30);
#endif
        if (peb && peb->Ldr) {
            const auto inLoadOrderList = (PLIST_ENTRY)peb->Ldr->Reserved2[1];
            const auto tableEntry = CONTAINING_RECORD(inLoadOrderList, LDR_DATA_TABLE_ENTRY, Reserved1[0]);
            auto pSizeOfImage = (PULONG)&tableEntry->Reserved3[1];
            *pSizeOfImage = (ULONG)((INT_PTR)tableEntry->DllBase + 0x100000);
        }
    }

    // =========================================================================
    // LAYER 3: Anti-Attach (from AntiDebugging AntiAttach + ReFo)
    //   - Patch DbgBreakPoint -> RET
    //   - Patch DbgUiRemoteBreakin -> RET (block debugger attach)
    //   NOTE: NtContinue patch REMOVED - it breaks ALL exception handling
    //   NOTE: JMP ExitProcess REMOVED - x64 relative offset overflows 32-bit
    // =========================================================================
    static void patch_anti_attach() {
        HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
        if (!hNtdll) return;

        // 1. Patch DbgBreakPoint -> RET (0xC3)
        auto pDbgBreakPoint = (BYTE*)GetProcAddress(hNtdll, "DbgBreakPoint");
        if (pDbgBreakPoint) {
            DWORD oldProt;
            if (VirtualProtect(pDbgBreakPoint, 2, PAGE_EXECUTE_READWRITE, &oldProt)) {
                pDbgBreakPoint[0] = 0xC3; // RET
                VirtualProtect(pDbgBreakPoint, 2, oldProt, &oldProt);
            }
        }

        // 2. Patch DbgUiRemoteBreakin -> RET (0xC3)
        //    Blocks debugger from attaching to process at runtime
        auto pDbgUiRemote = (BYTE*)GetProcAddress(hNtdll, "DbgUiRemoteBreakin");
        if (pDbgUiRemote) {
            DWORD oldProt;
            if (VirtualProtect(pDbgUiRemote, 4, PAGE_EXECUTE_READWRITE, &oldProt)) {
                pDbgUiRemote[0] = 0xC3; // RET
                VirtualProtect(pDbgUiRemote, 4, oldProt, &oldProt);
            }
        }
    }

    // =========================================================================
    // LAYER 4: .text Section Integrity Check (CRC32 - from AntiDebugging source)
    //   - Computes CRC32 hash of .text section at startup
    //   - Background thread periodically re-checks for code patches/hooks
    // =========================================================================
    struct TextSectionInfo {
        void* address = nullptr;
        uint32_t size = 0;
        uint32_t original_crc = 0;
        bool valid = false;
    };

    static TextSectionInfo g_textSection;

    static uint32_t compute_crc32(const void* data, size_t size) {
        uint32_t crc = 0;
        const uint8_t* ptr = (const uint8_t*)data;
        for (size_t i = 0; i < size; i++) {
            crc = _mm_crc32_u8(crc, ptr[i]);
        }
        return crc;
    }

    static void init_text_section_hash() {
        HMODULE hModule = GetModuleHandleW(NULL);
        if (!hModule) return;

        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return;

        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return;

        PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);
        for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++, section++) {
            if (memcmp(section->Name, ".text", 5) == 0) {
                g_textSection.address = (void*)((uintptr_t)hModule + section->VirtualAddress);
                g_textSection.size = section->Misc.VirtualSize;
                g_textSection.original_crc = compute_crc32(g_textSection.address, g_textSection.size);
                g_textSection.valid = true;
                break;
            }
        }
    }

    static bool check_text_integrity() {
        if (!g_textSection.valid) return true; // Can't check = assume OK
        uint32_t current_crc = compute_crc32(g_textSection.address, g_textSection.size);
        return (current_crc == g_textSection.original_crc);
    }

    // =========================================================================
    // LAYER 5: Process Blacklist Scanning (from Anti-Debugger-Protector-Loader)
    //   - Detects debuggers, disassemblers, network sniffers, memory editors
    // =========================================================================
    static bool scan_blacklisted_processes() {
        static const wchar_t* blacklist[] = {
            // Debuggers
            L"x64dbg.exe", L"x32dbg.exe", L"ollydbg.exe", L"OLLYDBG.exe",
            L"ida.exe", L"ida64.exe", L"idaq.exe", L"idaq64.exe",
            // Memory editors/dumpers
            L"cheatengine-x86_64.exe", L"Cheat Engine.exe",
            L"KsDumperClient.exe", L"KsDumper.exe",
            L"ProcessHacker.exe", L"procmon.exe", L"procexp.exe", L"procexp64.exe",
            L"HxD64.exe", L"HxD32.exe", L"HxD.exe",
            // Network sniffers
            L"Wireshark.exe", L"Fiddler.exe", L"Fiddler Everywhere.exe",
            L"HTTPDebuggerUI.exe", L"HTTPDebuggerSvc.exe",
            L"HTTP Debugger Windows Service (32 bit).exe",
            // Decompilers/Reversing
            L"snowman.exe", L"die.exe", L"Detect It Easy.exe",
            L"Xenos64.exe", L"Xenos.exe",
            L"dnSpy.exe", L"dotPeek32.exe", L"dotPeek64.exe",
            L"Ghidra.exe", L"ghidraRun.bat",
            // Monitoring
            L"Autoruns.exe", L"Autoruns64.exe",
            L"Regshot-x64-Unicode.exe",
            L"FolderChangesView.exe",
            L"ApiMonitor-x64.exe", L"ApiMonitor-x86.exe",
            // VM & Analysis (optional)
            L"vmtoolsd.exe", L"vmwaretray.exe",
        };

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;

        PROCESSENTRY32W pe = { sizeof(PROCESSENTRY32W) };
        bool found = false;
        if (Process32FirstW(snapshot, &pe)) {
            do {
                for (const auto& bl : blacklist) {
                    if (_wcsicmp(pe.szExeFile, bl) == 0) {
                        found = true;
                        break;
                    }
                }
                if (found) break;
            } while (Process32NextW(snapshot, &pe));
        }
        CloseHandle(snapshot);
        return found;
    }

    // =========================================================================
    // LAYER 6: Window Title Scanning (from Anti-Debugger-Protector-Loader)
    //   - FindWindow-based detection of known tool windows
    // =========================================================================
    static bool scan_window_titles() {
        static const wchar_t* titles[] = {
            L"IDA: Quick start", L"IDA -", L"IDA Pro",
            L"x32dbg", L"x64dbg", L"x32_dbg", L"x64_dbg",
            L"Memory Viewer", L"Cheat Engine",
            L"Cheat Engine 7.0", L"Cheat Engine 7.1", L"Cheat Engine 7.2",
            L"Cheat Engine 7.3", L"Cheat Engine 7.4", L"Cheat Engine 7.5",
            L"Process List",
            L"KsDumper",
            L"Fiddler Everywhere", L"Fiddler Classic", L"FiddlerCap", L"FiddlerCore",
            L"HTTP Debugger",
            L"OllyDbg",
            L"Scylla x86", L"Scylla x64",
            L"Detect It Easy",
            L"HxD",
            L"Snowman",
            L"Process Hacker",
            L"API Monitor",
            L"Wireshark",
        };

        for (const auto& title : titles) {
            HWND wnd = FindWindowW(NULL, title);
            if (wnd) return true;
        }
        return false;
    }

    // =========================================================================
    // LAYER 7: Driver/Device Detection
    //   - Checks for known debugger kernel drivers
    // =========================================================================
    static bool scan_debug_drivers() {
        static const wchar_t* devices[] = {
            L"\\\\.\\Dumper",
            L"\\\\.\\KsDumper",
            L"\\\\.\\ICEEXT",
            L"\\\\.\\CEDRIVER73",
            L"\\\\.\\NDBGMSG",
            L"\\\\.\\picodrv",
        };

        for (const auto& dev : devices) {
            HANDLE hFile = CreateFileW(dev, GENERIC_READ, FILE_SHARE_READ, NULL,
                                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                CloseHandle(hFile);
                return true;
            }
        }
        return false;
    }

    // =========================================================================
    // LAYER 8: Anti-DLL Injection Detection
    //   - Check for known analysis DLLs loaded in our process
    // =========================================================================
    static bool check_suspicious_modules() {
        static const char* suspicious[] = {
            "renderdoc.dll",
            "fraps.dll",
            "SbieDll.dll",     // Sandboxie
            // NOTE: dbghelp.dll removed - legitimately loaded by Windows/CRT
            "api_log.dll",     // API Monitor
            "dir_watch.dll",
            "pstorec.dll",
            "hooklib.dll",
        };

        for (const auto& mod : suspicious) {
            if (GetModuleHandleA(mod)) return true;
        }
        return false;
    }

    // =========================================================================
    // LAYER 9: Timing-based Anti-Debug
    //   - RDTSC timing check to detect single-stepping
    // =========================================================================
    static bool check_timing_attack() {
        LARGE_INTEGER freq, start, end;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);

        // Simple computation that should take < 1ms normally
        volatile int dummy = 0;
        for (int i = 0; i < 100; i++) dummy += i;

        QueryPerformanceCounter(&end);

        // If this simple loop takes more than 500ms, something is stepping through
        double elapsed_ms = ((double)(end.QuadPart - start.QuadPart) / freq.QuadPart) * 1000.0;
        return (elapsed_ms > 500.0);
    }

    // =========================================================================
    // LAYER 10: Kill Known Debug Processes (Aggressive)
    // =========================================================================
    static void kill_debug_processes() {
        static const wchar_t* killList[] = {
            L"taskkill /f /im HTTPDebuggerUI.exe >nul 2>&1",
            L"taskkill /f /im HTTPDebuggerSvc.exe >nul 2>&1",
            L"sc stop HTTPDebuggerPro >nul 2>&1",
            L"taskkill /FI \"IMAGENAME eq cheatengine*\" /IM * /F /T >nul 2>&1",
            L"taskkill /FI \"IMAGENAME eq httpdebugger*\" /IM * /F /T >nul 2>&1",
            L"taskkill /FI \"IMAGENAME eq processhacker*\" /IM * /F /T >nul 2>&1",
        };

        for (const auto& cmd : killList) {
            _wsystem(cmd);
        }
    }

    // =========================================================================
    // PROTECTION THREADS (using CreateThread - no std::thread dependency)
    // =========================================================================

    // Thread 1: Main protection loop (process/window/driver scan + API checks)
    static DWORD WINAPI protection_loop_thread(LPVOID) {
        while (true) {
            hide_thread_from_debugger();

            if (check_api_debugger()) debugger_detected();
            if (check_hardware_breakpoints()) debugger_detected();
            if (scan_exe && scan_blacklisted_processes()) debugger_detected();
            if (scan_title && scan_window_titles()) debugger_detected();
            if (scan_driver && scan_debug_drivers()) debugger_detected();
            if (check_suspicious_modules()) debugger_detected();

            static int timing_counter = 0;
            if (++timing_counter % 5 == 0) {
                if (check_timing_attack()) debugger_detected();
            }

            SleepEx(scan_detection_time, TRUE);
        }
        return 0;
    }

    // Thread 2: .text section integrity monitor
    static DWORD WINAPI integrity_monitor_thread(LPVOID) {
        Sleep(3000);
        init_text_section_hash();
        while (true) {
            Sleep(2000);
            if (!check_text_integrity()) debugger_detected();
        }
        return 0;
    }

    // Thread 3: Periodic kill of debug processes
    static DWORD WINAPI kill_loop_thread(LPVOID) {
        while (true) {
            Sleep(60000);
            kill_debug_processes();
        }
        return 0;
    }

    // =========================================================================
    // PUBLIC INTERFACE
    // =========================================================================

    // One-time initialization protections (call before main logic)
    static void init_protections() {
        // 1. Anti-Attach: Patch ntdll functions
        patch_anti_attach();

        // 2. Anti-Dump: Corrupt SizeOfImage in PEB
        corrupt_size_of_image();

        // 3. Hide main thread from debugger
        hide_thread_from_debugger();

        // 4. kill_debug_processes moved to background thread only
        //    (calling _wsystem during init blocks startup)

        // 5. Initial security check
        if (check_api_debugger() || check_hardware_breakpoints()) {
            debugger_detected();
        }
    }

    // Start all protection threads
    void start_protect() {
        // Run one-time protections first
        init_protections();

        // Launch background monitoring threads via CreateThread
        CloseHandle(CreateThread(NULL, 0, protection_loop_thread, NULL, 0, NULL));
        CloseHandle(CreateThread(NULL, 0, integrity_monitor_thread, NULL, 0, NULL));

        if (loop_killdbgr) {
            CloseHandle(CreateThread(NULL, 0, kill_loop_thread, NULL, 0, NULL));
        }
    }

}; // namespace protector
