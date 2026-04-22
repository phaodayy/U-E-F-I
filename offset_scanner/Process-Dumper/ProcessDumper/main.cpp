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

	printf("\n[*] Targeting: %s (Threshold: %.1f%%)\n", TargetProcess.c_str(), Threshold * 100.0f);

	if (!Dumper.Attach(TargetProcess)) {
		printf("[!] Failed to attach to %s. Check if game is running.\n", TargetProcess.c_str());
		system("pause");
		return 1;
	}

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
		printf("\r[*] Progress: %.1f%% (%llu/%llu pages)", 
			progress * 100.0f, 
			Dumper.GetDecryptedCount(), 
			Dumper.GetTotalPages());

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
