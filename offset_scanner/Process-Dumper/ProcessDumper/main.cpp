#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include "Dumper/ProcessDumper.hpp"
#include "Sdk/memory.hpp"

void print_header() {
	printf("================================================\n");
	printf("     Process-Dumper (Hyper-reV Version)       \n");
	printf("================================================\n\n");
}

int main(int argc, char** argv) {
	print_header();

	if (!DumperMemory::InitializeHyperInterface()) {
		printf("[!] Hyper-reV hypervisor not found or initialization failed!\n");
		system("pause");
		return 1;
	}
	printf("[*] Hyper-reV usermode bridge initialized\n");

	std::string TargetProcess;
	float Threshold = 1.0f;
	
	if (argc < 2) {
		printf("[?] Select a target process:\n");
		printf(" 1. PUBG (TslGame.exe)\n");
		printf(" 2. Explorer (explorer.exe) - for testing\n");
		printf(" 3. Other (Enter name manually)\n");
		printf("> ");
		
		int choice;
		std::cin >> choice;
		
		if (choice == 1) TargetProcess = "TslGame.exe";
		else if (choice == 2) TargetProcess = "explorer.exe";
		else {
			printf("Enter process name (e.g. notepad.exe): ");
			std::cin >> TargetProcess;
		}
	} else {
		TargetProcess = argv[1];
		if (argc >= 4 && std::string(argv[2]) == "-t") {
			Threshold = std::stof(argv[3]);
		}
	}

	ProcessDumper Dumper;
	Dumper.SetDecryptionThreshold(Threshold);

	printf("\n[*] Waiting for %s to start and stabilize...\n", TargetProcess.c_str());
	
	while (true) {
		if (Dumper.Attach(TargetProcess)) {
			if (Dumper.GetImageBase() != 0 && Dumper.GetImageBase() < 0xFFFF000000000000) {
				break;
			}
		}
		printf(".");
		Sleep(1000);
	}

	printf("\n[+] Attached: %s (PID: %lu). Base: 0x%llX\n", TargetProcess.c_str(), DumperMemory::g_ProcessId, Dumper.GetImageBase());

	printf("[*] Starting memory monitoring...\n");
	if (!Dumper.StartMonitoring()) {
		printf("[!] Failed to start monitoring.\n");
		system("pause");
		return 1;
	}

	printf("\n[+] Dumper is RUNNING. Press 'Q' to stop and save dump.\n");
	
	while (true) {
		if (GetAsyncKeyState('Q') & 0x8000) break;

		float progress = Dumper.GetDecryptionProgress();
		uint64_t readable = Dumper.GetDecryptedCount();
		uint64_t total = Dumper.GetTotalPages();
		uint64_t locked = total - readable;

		printf("\r[*] Memory: %llu/%llu (Locked: %llu) | Progress: %.1f%%  ", 
			readable, 
			total,
			locked,
			progress * 100.0f);

		if (progress >= Dumper.GetDecryptionThreshold()) {
			printf("\n[+] Target threshold reached (%.1f%%)\n", Dumper.GetDecryptionThreshold() * 100.0f);
			break;
		}
		Sleep(500);
	}

	printf("\n[*] Finalizing dump...\n");
	Dumper.DumpCurrent();
	
	std::string OutputFile = "dump_" + TargetProcess;
	if (Dumper.Rebuild(OutputFile)) {
		printf("[++] SUCCESS: Dump saved to %s\n", OutputFile.c_str());
	} else {
		printf("[!] FAILED to rebuild/save dump.\n");
	}

	system("pause");
	return 0;
}
