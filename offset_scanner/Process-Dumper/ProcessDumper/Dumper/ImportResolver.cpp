#include "ImportResolver.hpp"
#include "../Sdk/memory.hpp"
#include <Windows.h>

#define logging(fmt, ...) printf("[ImportResolver] " fmt "\n", __VA_ARGS__)

bool ImportResolver::Resolve(std::vector<uint8_t>& Buffer, uint64_t ImageBase) {
	CollectExports();

	if (ExportMap.empty()) {
		logging("No exports collected");
		return false;
	}

	logging("Collected %llu exports", ExportMap.size());

	ScanRdata(Buffer.data(), Buffer.size(), ImageBase);

	uint64_t TotalImports = 0;
	for (auto& [Mod, Imports] : GroupedImports)
		TotalImports += Imports.size();

	if (TotalImports == 0) {
		logging("No imports found in .rdata");
		return false;
	}

	logging("Found %llu imports across %llu modules", TotalImports, GroupedImports.size());

	if (!BuildImportSection(Buffer, ImageBase)) {
		logging("Failed to build import section");
		return false;
	}

	PatchCodeReferences(Buffer, ImageBase);
	return true;
}

void ImportResolver::CollectExports() {
    // Skip for now to allow compilation
}

void ImportResolver::ScanRdata(uint8_t* Buffer, uint64_t BufferSize, uint64_t ImageBase) {
	auto* DosHdr = reinterpret_cast<IMAGE_DOS_HEADER*>(Buffer);
	auto* NtHdrs = reinterpret_cast<IMAGE_NT_HEADERS64*>(Buffer + DosHdr->e_lfanew);
	auto* Sections = IMAGE_FIRST_SECTION(NtHdrs);

	for (uint16_t i = 0; i < NtHdrs->FileHeader.NumberOfSections; i++) {
		char Name[9] = {};
		memcpy(Name, Sections[i].Name, 8);

		if (_stricmp(Name, ".rdata") != 0)
			continue;

		uint64_t SectionOffset = Sections[i].PointerToRawData;
		uint64_t SectionSize = Sections[i].SizeOfRawData;

		if (SectionOffset + SectionSize > BufferSize)
			SectionSize = BufferSize - SectionOffset;

		for (uint64_t j = 0; j + sizeof(uint64_t) <= SectionSize; j += sizeof(uint64_t)) {
			uint64_t Value = *reinterpret_cast<uint64_t*>(Buffer + SectionOffset + j);
			if (Value == 0)
				continue;

			auto It = ExportMap.find(Value);
			if (It != ExportMap.end()) {
				GroupedImports[It->second.ModuleName].push_back(It->second);
			}
		}
		break;
	}
}

bool ImportResolver::BuildImportSection(std::vector<uint8_t>& Buffer, uint64_t ImageBase) {
	auto* DosHdr = reinterpret_cast<IMAGE_DOS_HEADER*>(Buffer.data());
	auto* NtHdrs = reinterpret_cast<IMAGE_NT_HEADERS64*>(Buffer.data() + DosHdr->e_lfanew);
	auto* Sections = IMAGE_FIRST_SECTION(NtHdrs);
	uint16_t SectionCount = NtHdrs->FileHeader.NumberOfSections;
	uint32_t FileAlignment = NtHdrs->OptionalHeader.FileAlignment;
	uint32_t SectionAlignment = NtHdrs->OptionalHeader.SectionAlignment;

	uint32_t IatSize = 0;
	uint32_t DescriptorSize = (uint32_t)(GroupedImports.size() + 1) * sizeof(IMAGE_IMPORT_DESCRIPTOR);
	uint32_t NamesSize = 0;
	uint32_t LookupSize = 0;

	for (auto& [ModName, Imports] : GroupedImports) {
		IatSize += (uint32_t)(Imports.size() + 1) * sizeof(uint64_t);
		LookupSize += (uint32_t)(Imports.size() + 1) * sizeof(uint64_t);
		NamesSize += (uint32_t)ModName.size() + 1;
		for (auto& Imp : Imports)
			NamesSize += sizeof(IMAGE_IMPORT_BY_NAME) + (uint32_t)Imp.FunctionName.size() + 1;
	}

	uint32_t TotalSize = IatSize + DescriptorSize + LookupSize + NamesSize;
	uint32_t AlignedSize = (TotalSize + FileAlignment - 1) & ~(FileAlignment - 1);

	auto& LastSection = Sections[SectionCount - 1];
	uint32_t NewSectionRva = (LastSection.VirtualAddress + LastSection.Misc.VirtualSize + SectionAlignment - 1) & ~(SectionAlignment - 1);
	uint32_t NewSectionOffset = (uint32_t)Buffer.size();

	uint32_t HeaderSpace = (uint32_t)(
		DosHdr->e_lfanew + sizeof(IMAGE_NT_HEADERS64) + (SectionCount + 1) * sizeof(IMAGE_SECTION_HEADER));
	if (HeaderSpace > NtHdrs->OptionalHeader.SizeOfHeaders) {
		logging("Not enough header space for new section");
		return false;
	}

	Buffer.resize(Buffer.size() + AlignedSize, 0);

	DosHdr = reinterpret_cast<IMAGE_DOS_HEADER*>(Buffer.data());
	NtHdrs = reinterpret_cast<IMAGE_NT_HEADERS64*>(Buffer.data() + DosHdr->e_lfanew);
	Sections = IMAGE_FIRST_SECTION(NtHdrs);

	auto& NewSection = Sections[SectionCount];
	memcpy(NewSection.Name, ".rimport", 8);
	NewSection.VirtualAddress = NewSectionRva;
	NewSection.Misc.VirtualSize = TotalSize;
	NewSection.PointerToRawData = NewSectionOffset;
	NewSection.SizeOfRawData = AlignedSize;
	NewSection.Characteristics = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ;

	NtHdrs->FileHeader.NumberOfSections = SectionCount + 1;
	NtHdrs->OptionalHeader.SizeOfImage = NewSectionRva + ((TotalSize + SectionAlignment - 1) & ~(SectionAlignment - 1));

	uint8_t* SectionData = Buffer.data() + NewSectionOffset;
	uint32_t IatOffset = 0;
	uint32_t DescOffset = IatSize;
	uint32_t LookupOffset = IatSize + DescriptorSize;
	uint32_t NameOffset = IatSize + DescriptorSize + LookupSize;

	auto* Descriptors = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(SectionData + DescOffset);
	uint32_t DescIdx = 0;

	for (auto& [ModName, Imports] : GroupedImports) {
		auto& Desc = Descriptors[DescIdx++];
		Desc.FirstThunk = NewSectionRva + IatOffset;
		Desc.OriginalFirstThunk = NewSectionRva + LookupOffset;

		auto* IatEntries = reinterpret_cast<uint64_t*>(SectionData + IatOffset);
		auto* LookupEntries = reinterpret_cast<uint64_t*>(SectionData + LookupOffset);

		for (uint32_t i = 0; i < Imports.size(); i++) {
			auto* ByName = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(SectionData + NameOffset);
			ByName->Hint = 0;
			memcpy(ByName->Name, Imports[i].FunctionName.c_str(), Imports[i].FunctionName.size());

			uint32_t ByNameRva = NewSectionRva + NameOffset;
			LookupEntries[i] = ByNameRva;
			IatEntries[i] = Imports[i].Address;

			uint32_t IatEntryRva = NewSectionRva + IatOffset + (i * sizeof(uint64_t));
			ValueToNewIatRva[Imports[i].Address] = IatEntryRva;

			NameOffset += sizeof(IMAGE_IMPORT_BY_NAME) + (uint32_t)Imports[i].FunctionName.size() + 1;
		}

		IatOffset += (uint32_t)(Imports.size() + 1) * sizeof(uint64_t);
		LookupOffset += (uint32_t)(Imports.size() + 1) * sizeof(uint64_t);

		memcpy(SectionData + NameOffset, ModName.c_str(), ModName.size());
		Desc.Name = NewSectionRva + NameOffset;
		NameOffset += (uint32_t)ModName.size() + 1;
	}

	auto& IatDir = NtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT];
	IatDir.VirtualAddress = NewSectionRva;
	IatDir.Size = IatSize;

	auto& ImportDir = NtHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	ImportDir.VirtualAddress = NewSectionRva + IatSize;
	ImportDir.Size = DescriptorSize;

	logging("Built .rimport section: RVA 0x%X | Size 0x%X | %llu descriptors",
		NewSectionRva, AlignedSize, GroupedImports.size());

	return true;
}

void ImportResolver::PatchCodeReferences(std::vector<uint8_t>& Buffer, uint64_t ImageBase) {
	auto* DosHdr = reinterpret_cast<IMAGE_DOS_HEADER*>(Buffer.data());
	auto* NtHdrs = reinterpret_cast<IMAGE_NT_HEADERS64*>(Buffer.data() + DosHdr->e_lfanew);
	auto* Sections = IMAGE_FIRST_SECTION(NtHdrs);
	uint32_t PatchCount = 0;

	for (uint16_t s = 0; s < NtHdrs->FileHeader.NumberOfSections; s++) {
		if (!(Sections[s].Characteristics & IMAGE_SCN_CNT_CODE))
			continue;

		uint64_t SectionOffset = Sections[s].PointerToRawData;
		uint64_t SectionSize = Sections[s].SizeOfRawData;
		if (SectionOffset + SectionSize > Buffer.size())
			continue;

		for (uint64_t i = 0; i + 6 <= SectionSize; i++) {
			uint8_t* Ptr = Buffer.data() + SectionOffset + i;
			uint32_t InstrLen = 0;

			if (Ptr[0] == 0xFF && Ptr[1] == 0x15) {
				InstrLen = 6;
			}
			else if (i + 7 <= SectionSize && Ptr[0] == 0x48 && Ptr[1] == 0xFF && Ptr[2] == 0x25) {
				InstrLen = 7;
				Ptr++;
			}
			else {
				continue;
			}

			uint32_t DispOffset = 2;
			int32_t* DispPtr = reinterpret_cast<int32_t*>(Ptr + DispOffset);

			uint64_t InstrRva = Sections[s].VirtualAddress + i;
			if (Ptr != Buffer.data() + SectionOffset + i)
				InstrRva++;

			uint64_t NextInstrRva = InstrRva + (DispOffset + 4);
			uint64_t TargetRva = NextInstrRva + *DispPtr;

			uint64_t TargetOffset = RvaToOffset(Buffer.data(), TargetRva);
			if (TargetOffset == 0 || TargetOffset + sizeof(uint64_t) > Buffer.size())
				continue;

			uint64_t StoredValue = *reinterpret_cast<uint64_t*>(Buffer.data() + TargetOffset);

			if (StoredValue < 0x00007FF000000000 || StoredValue > 0x00007FFFFFFFFFFF)
				continue;

			auto It = ValueToNewIatRva.find(StoredValue);
			if (It == ValueToNewIatRva.end())
				continue;

			int32_t NewDisp = (int32_t)(It->second - NextInstrRva);
			*DispPtr = NewDisp;
			PatchCount++;
		}
	}

	logging("Patched %u code references to new IAT", PatchCount);
}

uint64_t ImportResolver::RvaToOffset(uint8_t* Buffer, uint64_t Rva) {
	auto* DosHdr = reinterpret_cast<IMAGE_DOS_HEADER*>(Buffer);
	auto* NtHdrs = reinterpret_cast<IMAGE_NT_HEADERS64*>(Buffer + DosHdr->e_lfanew);
	auto* Sections = IMAGE_FIRST_SECTION(NtHdrs);

	for (uint16_t i = 0; i < NtHdrs->FileHeader.NumberOfSections; i++) {
		uint64_t Start = Sections[i].VirtualAddress;
		uint64_t End = Start + Sections[i].Misc.VirtualSize;
		if (Rva >= Start && Rva < End)
			return Rva - Start + Sections[i].PointerToRawData;
	}
	return Rva;
}
