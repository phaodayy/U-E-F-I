#include "ProcessDumper.hpp"
#include <fstream>
#include <filesystem>
#include "../Sdk/memory.hpp"

ProcessDumper::ProcessDumper()
	: ImageBase(0), ImageSize(0), Attached(false), DecryptionThreshold(1.0f) {}

ProcessDumper::~ProcessDumper() {
	Monitor.Stop();
}

bool ProcessDumper::Attach(const std::string& ProcessName) {
	this->ProcessName = ProcessName;

	if (!DumperMemory::Attach(ProcessName)) {
		printf("[!] Failed to attach to %s via Hyper-V\n", ProcessName.c_str());
		return false;
	}

	ImageBase = DumperMemory::g_BaseAddress;
    
    uint16_t mz = DumperMemory::Read<uint16_t>(ImageBase);
    if (mz != 0x5A4D) {
        printf("[!] Invalid MZ header at 0x%llX\n", ImageBase);
        return false;
    }

    uint32_t pe_offset = DumperMemory::Read<uint32_t>(ImageBase + 0x3C);
    ImageSize = DumperMemory::Read<uint32_t>(ImageBase + pe_offset + 0x50); // SizeOfImage

	if (!ImageBase || !ImageSize) {
		printf("[!] Failed to resolve Image info for %s\n", ProcessName.c_str());
		return false;
	}

	printf("[+] Attached: 0x%llX (Size: 0x%llX)\n", ImageBase, ImageSize);

	DumpBuffer.resize(ImageSize, 0);

	if (!ReadInitialImage())
		logging("Warning: Initial image read incomplete, monitor will fill in the rest");

	Attached = true;
	return true;
}

bool ProcessDumper::StartMonitoring() {
	if (!Attached) {
		logging("Not attached");
		return false;
	}

	if (!Monitor.Init(ImageBase, ImageSize)) {
		logging("Failed to init page monitor");
		return false;
	}

	Monitor.SetCallback([this](const DecryptedRegion& Region) {
		OnPageDecrypted(Region);
	});

	Monitor.Start();
	logging("Monitor started - scanning pages via Hyper-V bridge");
	return true;
}

bool ProcessDumper::DumpCurrent() {
	Monitor.Stop();
	logging("Monitor stopped");
	return true;
}

bool ProcessDumper::Rebuild(const std::string& OutputPath) {
	std::lock_guard<std::mutex> Lock(DumpMutex);

	if (!Rebuilder.LoadFromBuffer(DumpBuffer.data(), DumpBuffer.size(), ImageBase)) {
		logging("Failed to parse dump buffer");
		return false;
	}

	Rebuilder.FixHeaders();
	Rebuilder.FixSectionHeaders();
	Rebuilder.FixImportDirectory();

	ExceptionFixer::Fix(DumpBuffer.data(), DumpBuffer.size());

	Resolver.Resolve(DumpBuffer, ImageBase);

	if (!Rebuilder.LoadFromBuffer(DumpBuffer.data(), DumpBuffer.size(), ImageBase)) {
		logging("Failed to reparse after import resolution");
		return false;
	}

	Rebuilder.NullRelocations();

	return Rebuilder.SaveToDisk(OutputPath);
}

void ProcessDumper::SetDecryptionThreshold(float Threshold) {
	DecryptionThreshold = Threshold;
	if (DecryptionThreshold < 0.0f) DecryptionThreshold = 0.0f;
	if (DecryptionThreshold > 1.0f) DecryptionThreshold = 1.0f;
}

uint64_t ProcessDumper::GetImageBase() const { return ImageBase; }
uint64_t ProcessDumper::GetImageSize() const { return ImageSize; }
uint64_t ProcessDumper::GetDecryptedCount() const { return Monitor.GetDecryptedCount(); }
uint64_t ProcessDumper::GetTotalPages() const { return Monitor.GetTotalPages(); }
bool ProcessDumper::IsMonitoring() const { return Monitor.IsRunning(); }

float ProcessDumper::GetDecryptionProgress() const {
	uint64_t Total = Monitor.GetTotalPages();
	if (Total == 0) return 0.0f;
	return (float)Monitor.GetDecryptedCount() / (float)Total;
}

float ProcessDumper::GetDecryptionThreshold() const {
	return DecryptionThreshold;
}

void ProcessDumper::OnPageDecrypted(const DecryptedRegion& Region) {
	std::lock_guard<std::mutex> Lock(DumpMutex);

	uint64_t Offset = Region.BaseAddress - ImageBase;
	if (Offset + Region.Size > DumpBuffer.size())
		return;

	memcpy(DumpBuffer.data() + Offset, Region.Data.data(), Region.Size);
}

bool ProcessDumper::ReadInitialImage() {
	constexpr uint64_t ChunkSize = 0x1000;
	uint64_t PagesRead = 0;
	uint64_t TotalPages = ImageSize / ChunkSize;

	std::string DiskPath = GetTargetFilePath();
	std::ifstream DiskFile;
	std::vector<uint8_t> DiskBuffer;

	if (!DiskPath.empty()) {
		DiskFile.open(DiskPath, std::ios::binary);
		if (DiskFile.is_open()) {
			DiskFile.seekg(0, std::ios::end);
			auto FileSize = DiskFile.tellg();
			DiskFile.seekg(0, std::ios::beg);
			DiskBuffer.resize(FileSize);
			DiskFile.read(reinterpret_cast<char*>(DiskBuffer.data()), FileSize);
			DiskFile.close();
			logging("Loaded on-disk PE fallback: %s (%llu bytes)", DiskPath.c_str(), DiskBuffer.size());
		}
	}

	for (uint64_t Offset = 0; Offset < ImageSize; Offset += ChunkSize) {
		uint64_t ReadSize = min(ChunkSize, ImageSize - Offset);

		bool Ok = DumperMemory::ReadMemory(
			ImageBase + Offset,
			DumpBuffer.data() + Offset,
			ReadSize);

		if (Ok) {
			PagesRead++;
			continue;
		}

		if (!DiskBuffer.empty() && Offset + ReadSize <= DiskBuffer.size()) {
			memcpy(DumpBuffer.data() + Offset, DiskBuffer.data() + Offset, ReadSize);
			PagesRead++;
		}
	}

	logging("Initial read: %llu / %llu pages", PagesRead, TotalPages);
	return PagesRead > 0;
}

bool ProcessDumper::ReadFromDisk(uint64_t Offset, uint64_t Size, uint8_t* Dest) {
	std::string Path = GetTargetFilePath();
	if (Path.empty())
		return false;

	std::ifstream File(Path, std::ios::binary);
	if (!File.is_open())
		return false;

	File.seekg(Offset);
	File.read(reinterpret_cast<char*>(Dest), Size);
	return File.good();
}

std::string ProcessDumper::GetTargetFilePath() {
	// Hyper-V mode: avoid OpenProcess/QueryFullProcessImageNameA IOC.
	// Keep disk fallback optional by probing local working directory only.
	if (ProcessName.empty())
		return "";

	std::filesystem::path candidate = ProcessName;
	if (!candidate.has_extension())
		candidate += ".exe";

	if (std::filesystem::exists(candidate))
		return candidate.string();

	return "";
}
