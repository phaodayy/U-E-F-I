#include "PageMonitor.hpp"
#include "../Sdk/memory.hpp"
#include <iostream>

#define logging(fmt, ...) printf("[PageMonitor] " fmt "\n", __VA_ARGS__)

PageMonitor::PageMonitor()
	: ImageBase(0), ImageSize(0), Running(false), DecryptedCount(0), TotalPages(0), OnDecrypted(nullptr) {}

PageMonitor::~PageMonitor() {
	Stop();
}

bool PageMonitor::Init(uint64_t ImageBase, uint64_t ImageSize) {
	this->ImageBase = ImageBase;
	this->ImageSize = ImageSize;

	constexpr uint64_t PageSize = 0x1000;
	TotalPages = (ImageSize + PageSize - 1) / PageSize;

	Pages.resize(TotalPages);
	for (uint64_t i = 0; i < TotalPages; i++) {
		Pages[i].Address = ImageBase + (i * PageSize);
		Pages[i].Size = PageSize;
		Pages[i].State = PageState::Unknown;
		Pages[i].ContentHash = 0;
		Pages[i].LastReadSuccessful = false;
	}

	ReadBuffer.resize(PageSize);

	logging("Initialized monitor: 0x%llX | %llu pages", ImageBase, TotalPages);
	return true;
}

void PageMonitor::SetCallback(DecryptionCallback Callback) {
	OnDecrypted = Callback;
}

void PageMonitor::Start() {
	if (Running)
		return;
	Running = true;
	Worker = std::thread(&PageMonitor::MonitorThread, this);
}

void PageMonitor::Stop() {
	Running = false;
	if (Worker.joinable())
		Worker.join();
}

bool PageMonitor::IsRunning() const {
	return Running;
}

uint64_t PageMonitor::GetDecryptedCount() const {
	return DecryptedCount;
}

uint64_t PageMonitor::GetTotalPages() const {
	return TotalPages;
}

void PageMonitor::MonitorThread() {
	constexpr uint64_t PageSize = 0x1000;
	std::vector<uint8_t> LocalBuffer(PageSize);
	bool BaselineCaptured = false;
	uint64_t CurrentReadableCount = 0;

	while (Running) {
		for (uint64_t i = 0; i < TotalPages && Running; i++) {
			TrackedPage& Page = Pages[i];

			if (Page.State == PageState::Decrypted)
				continue;

			memset(LocalBuffer.data(), 0, PageSize);
			const bool ReadOk = DumperMemory::ReadMemory(Page.Address, LocalBuffer.data(), PageSize);
			if (!ReadOk) {
				Page.LastReadSuccessful = false;
				continue;
			}

			bool Empty = IsPageEmpty(LocalBuffer.data(), PageSize);
			uint64_t Hash = Empty ? 0 : HashPage(LocalBuffer.data(), PageSize);

			if (!BaselineCaptured) {
				Page.LastReadSuccessful = true;
				Page.State = Empty ? PageState::Empty : PageState::Unknown;
				Page.ContentHash = Hash;
				continue;
			}

			const bool TransitionedToReadable = (!Page.LastReadSuccessful && ReadOk);
			const bool TransitionedFromEmpty = (Page.State == PageState::Empty && !Empty);
			const bool ContentChanged = (!Empty && Page.ContentHash != 0 && Hash != Page.ContentHash);
			Page.LastReadSuccessful = true;

			if (Empty) {
				Page.State = PageState::Empty;
				Page.ContentHash = 0;
				continue;
			}

			Page.ContentHash = Hash;
			if (TransitionedToReadable || TransitionedFromEmpty || ContentChanged) {
				Page.State = PageState::Decrypted;
				DecryptedRegion Region;
				Region.BaseAddress = Page.Address;
				Region.Size = PageSize;
				Region.Data.assign(LocalBuffer.begin(), LocalBuffer.end());

				if (OnDecrypted) {
					OnDecrypted(Region);
				}
			}

			// Update atomic count for UI (Count all non-empty readable pages)
			if (Page.State != PageState::Empty && Page.LastReadSuccessful) {
				CurrentReadableCount++;
			}
		}
		DecryptedCount = CurrentReadableCount;
		CurrentReadableCount = 0;

		if (!BaselineCaptured)
			BaselineCaptured = true;

		Sleep(1); // Sleep only AFTER a full cycle of all pages
	}
}

uint64_t PageMonitor::HashPage(const uint8_t* Data, uint64_t Size) {
	uint64_t Hash = 0x517CC1B727220A95;
	for (uint64_t i = 0; i < Size; i++) {
		Hash ^= Data[i];
		Hash *= 0x5BD1E995;
		Hash ^= Hash >> 15;
	}
	return Hash;
}

bool PageMonitor::IsPageEmpty(const uint8_t* Data, uint64_t Size) {
	const uint64_t* QuadData = reinterpret_cast<const uint64_t*>(Data);
	uint64_t QuadCount = Size / sizeof(uint64_t);

	for (uint64_t i = 0; i < QuadCount; i++) {
		if (QuadData[i] != 0)
			return false;
	}
	return true;
}
