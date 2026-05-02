#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <winternl.h>
#include <fstream>

#include <structures/process.h>
#include <security/unified_auth.hpp>
#include "gui/menu.h"
#include "overlay/overlay_menu.hpp"
#include "sdk/context.hpp"
#include "sdk/offsets.hpp"
#include "sdk/scanner.hpp"
#include "sdk/skCrypt.h"
#include "sdk/protector/protector.hpp"
#include <time.h>


void SetConsoleColor(int color) {
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void TypewriterPrint(const std::string &text, int delay_ms = 20, int color = 7) {
  SetConsoleColor(color);
  for (char c : text) {
    std::cout << c << std::flush;
    Sleep(delay_ms);
  }
  SetConsoleColor(7);
}

void RandomizeConsoleTitle() {
  char rand_title[16] = {0};
  for (int i = 0; i < 15; i++)
    rand_title[i] = (rand() % 26) + 'a';
  SetConsoleTitleA(rand_title);
}

#include "sdk/driver.hpp"

DWORD GetProcessIdByName(const wchar_t *name) {
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snap == INVALID_HANDLE_VALUE) return 0;
  PROCESSENTRY32W pe = {sizeof(pe)};
  DWORD pid = 0;
  if (Process32FirstW(snap, &pe)) {
    do {
      if (_wcsicmp(pe.szExeFile, name) == 0) {
        pid = pe.th32ProcessID;
        break;
      }
    } while (Process32NextW(snap, &pe));
  }
  CloseHandle(snap);
  return pid;
}

// =============================================================================
// GZ-SECURITY SUITE V3.0 (from PUBG-2)
// =============================================================================

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
    const wchar_t* blacklisted[] = { skCrypt(L"ida64.exe"), skCrypt(L"x64dbg.exe"), skCrypt(L"cheatengine-x86_64.exe"), skCrypt(L"Fiddler.exe"), skCrypt(L"wireshark.exe"), skCrypt(L"HTTPDebuggerSvc.exe"), skCrypt(L"ida.exe"), skCrypt(L"x32dbg.exe") };
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W pe = {sizeof(pe)};
    if (Process32FirstW(snap, &pe)) {
        do {
            for (const auto& proc : blacklisted) {
                if (_wcsicmp(pe.szExeFile, proc) == 0) {
                    CloseHandle(snap);
                    return true;
                }
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
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
        // [7] ProcessDebugPort
        DWORD_PTR debugPort = 0;
        if (ntqip(GetCurrentProcess(), (PROCESSINFOCLASS)7, &debugPort, sizeof(debugPort), NULL) == 0 && debugPort != 0)
            return false;

        // [0x1E] ProcessDebugObjectHandle
        HANDLE debugObject = NULL;
        if (ntqip(GetCurrentProcess(), (PROCESSINFOCLASS)0x1E, &debugObject, sizeof(debugObject), NULL) == 0 && debugObject != NULL)
            return false;

        // [0x1F] ProcessDebugFlags (Should be 1 if NO debugger, 0 if debugger)
        DWORD debugFlags = 1;
        if (ntqip(GetCurrentProcess(), (PROCESSINFOCLASS)0x1F, &debugFlags, sizeof(debugFlags), NULL) == 0 && debugFlags == 0)
            return false;
    }

    // 3. Blacklist Window Check
    if (GetModuleHandleA(skCrypt("renderdoc.dll"))) return false;
    if (FindWindowA(skCrypt("Window"), skCrypt("x64dbg"))) return false;
    if (FindWindowA(skCrypt("Window"), skCrypt("Cheat Engine"))) return false;
    
    if (CheckHardwareBreakpoints()) return false;
    if (CheckBlacklistedProcesses()) return false;
    
    return true;
}

int main(int argc, char *argv[]) {
  // Initialize Security Protector
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

  srand((unsigned int)GetTickCount64());
  RandomizeConsoleTitle();

  if (!Driver::Initialize()) {
      SetConsoleColor(12);
      std::cout << skCrypt("[-] Hypervisor interface not found! Ensure the UEFI hypervisor is running.") << std::endl;
      SetConsoleColor(7);
      Sleep(5000);
      return 1;
  }
  
  system("cls");

  // Load and display version first
  VDataOffsets::LdrpInitializeProcessConfig();
  VDataOffsets::LdrpLoadLibraryConfigLocal();
  SetConsoleColor(11); // Light blue
  std::cout << skCrypt(">>> SYSTEM NODE v") << VDataOffsets::Version() << skCrypt(" <<<") << std::endl << std::endl;
  SetConsoleColor(7);

  TypewriterPrint("[", 10, 8);
  TypewriterPrint("1", 10, 11);
  TypewriterPrint("] ", 10, 8);
  TypewriterPrint("Checking driver...", 30, 7);
  std::cout << std::endl;

  query_process_data_packet test_packet = {};
  if (!Driver::QueryProcess(GetCurrentProcessId(), &test_packet)) {
    SetConsoleColor(12);
    std::cout << skCrypt("[-] Driver not found! Please run 3.GZ-LoaderDriver.exe first.") << std::endl;
    SetConsoleColor(7);
    std::cout << "\nPress Enter to exit..." << std::endl;
    std::cin.get();
    return 1;
  }

  SetConsoleColor(10);
  std::cout << "[+] Driver OK" << std::endl;
  SetConsoleColor(7);
  Sleep(300);

  // Authentication Part (shared UnifiedTelemetryNode-style flow)
  if (!UnifiedAuth::EnsureActiveLicenseForConsole(skCrypt("key.txt"))) {
    std::cout << skCrypt("[!] Authentication failed: ") << UnifiedAuth::g_Auth.license_error << std::endl;
    Sleep(3000);
    return 1;
  }
  std::cout << skCrypt("[+] License authenticated successfully!") << std::endl;

  VDataOffsets::LdrpInitializeProcessConfig();
  
  // Start background heartbeat to monitor expiration (Unified Style)
  std::thread([]() {
      UnifiedAuth::LicenseHeartbeatLoop();
  }).detach();

  TypewriterPrint("\n[", 10, 8);
  TypewriterPrint("2", 10, 11);
  TypewriterPrint("] ", 10, 8);
  TypewriterPrint("Waiting for VData...", 30, 14);

  DWORD pid = 0;
  int dots = 0;
  while (!pid) {
    pid = GetProcessIdByName(skCrypt(L"VALORANT-Win64-Shipping.exe"));
    if (!pid) {
      SetConsoleColor(14); std::cout << "."; SetConsoleColor(7);
      dots++; if (dots > 3) { std::cout << "\b\b\b\b    \b\b\b\b"; dots = 0; }
      Sleep(500);
    }
  }
  std::cout << std::endl;

  SetConsoleColor(10);
  std::cout << "[+] Target found (PID: " << pid << ")" << std::endl;
  SetConsoleColor(7);

  // Offset Fetching
  if (!VDataOffsets::LdrpLoadLibraryConfig(2)) {
      std::cout << skCrypt("[-] Failed to synchronize game data.") << std::endl;
      Sleep(5000); return 1;
  }

  TypewriterPrint("\n[", 10, 8);
  TypewriterPrint("3", 10, 11);
  TypewriterPrint("] ", 10, 8);
  TypewriterPrint("Connecting to data source...", 30, 7);
  std::cout << std::endl;

  query_process_data_packet qdata = {};
  if (Driver::QueryProcess(pid, &qdata)) {
    SetConsoleColor(10);
    std::cout << skCrypt("[+] Secure Link: ESTABLISHED") << std::endl;
    SetConsoleColor(8);
    std::cout << skCrypt("    Version: ") << VDataOffsets::Version() << std::endl;
    std::cout << skCrypt("    Base: 0x") << std::hex << (uint64_t)qdata.base_address << std::dec << std::endl;
    std::cout << skCrypt("    CR3:  0x") << std::hex << qdata.cr3 << std::dec << std::endl;
    std::cout << skCrypt("    PEB:  0x") << std::hex << (uint64_t)qdata.peb << std::dec << std::endl;
    SetConsoleColor(7);
  } else {
    SetConsoleColor(12);
    std::cout << "\n[-] Driver communication FAILED!" << std::endl;
    SetConsoleColor(7);
    std::cin.get();
    return 1;
  }

  uint64_t base = (uint64_t)qdata.base_address;
  uint16_t mz = Driver::Read<uint16_t>(pid, base);

  if (mz == 0x5A4D) {
    TypewriterPrint("\n[", 10, 8);
    TypewriterPrint("4", 10, 11);
    TypewriterPrint("] ", 10, 8);
    TypewriterPrint("Game synchronization: OK", 30, 14);
    std::cout << std::endl;

    TypewriterPrint("\n[", 10, 8);
    TypewriterPrint("5", 10, 11);
    TypewriterPrint("] ", 10, 8);
    TypewriterPrint("Connecting to engine", 30, 7);

    bool engineConnected = false;
    int dots2 = 0;
    while (!engineConnected) {
      if (VDataContext::Initialize(pid, base)) {
        engineConnected = true;
      } else {
        SetConsoleColor(14); std::cout << "."; SetConsoleColor(7);
        dots2++; if (dots2 > 3) { std::cout << "\b\b\b\b    \b\b\b\b"; dots2 = 0; }
        Sleep(1000);
      }
    }
    std::cout << std::endl;

    SetConsoleColor(10); std::cout << "[+] Engine ready" << std::endl; SetConsoleColor(7);

    TypewriterPrint("\n[", 10, 8);
    TypewriterPrint("6", 10, 11);
    TypewriterPrint("] ", 10, 8);
    TypewriterPrint("Initializing Overlay...", 30, 7);
    std::cout << std::endl;

    g_Menu.Initialize(nullptr);
    SetConsoleColor(10); std::cout << "[+] Overlay active" << std::endl; SetConsoleColor(7);

    std::thread update_thread([&]() {
      while (true) {
        VDataContext::UpdateGameData();
        Sleep(5);
      }
    });
    update_thread.detach();

    std::cout << skCrypt("\n[+] SYSTEM ACTIVE | INSERT: Menu | F12: Panic Exit") << std::endl;

    while (true) {
      g_Menu.RenderFrame();
      Sleep(1);
      if (GetAsyncKeyState(VK_F12) & 0x8000) break;
    }
    g_Menu.Shutdown();
  }

  return 0;
}
