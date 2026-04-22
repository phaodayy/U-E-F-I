#include "pe_image.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <string_view>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace {
bool ReadFileToBuffer(const std::string& path, std::vector<std::uint8_t>& out, std::string& error) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        error = "Failed to open file: " + path;
        return false;
    }

    ifs.seekg(0, std::ios::end);
    const std::streamoff size = ifs.tellg();
    if (size <= 0) {
        error = "File is empty or unreadable: " + path;
        return false;
    }

    ifs.seekg(0, std::ios::beg);
    out.resize(static_cast<std::size_t>(size));
    if (!ifs.read(reinterpret_cast<char*>(out.data()), size)) {
        error = "Failed to read file bytes: " + path;
        return false;
    }
    return true;
}

std::string TrimSectionName(const IMAGE_SECTION_HEADER& section) {
    const char* raw = reinterpret_cast<const char*>(section.Name);
    std::size_t length = 0;
    while (length < IMAGE_SIZEOF_SHORT_NAME && raw[length] != '\0') {
        ++length;
    }
    return std::string(raw, raw + length);
}
} // namespace

bool PeImage::LoadFromFile(const std::string& path, std::string& error) {
    path_ = path;
    bytes_.clear();
    sections_.clear();
    size_of_headers_ = 0;

    if (!ReadFileToBuffer(path, bytes_, error)) {
        return false;
    }

    if (bytes_.size() < sizeof(IMAGE_DOS_HEADER)) {
        error = "Not a valid PE file (too small): " + path;
        return false;
    }

    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(bytes_.data());
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        error = "Not a valid PE file (DOS signature mismatch): " + path;
        return false;
    }

    if (dos->e_lfanew <= 0 || static_cast<std::size_t>(dos->e_lfanew) + sizeof(IMAGE_NT_HEADERS64) > bytes_.size()) {
        error = "Not a valid PE file (invalid NT header offset): " + path;
        return false;
    }

    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(bytes_.data() + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        error = "Not a valid PE file (NT signature mismatch): " + path;
        return false;
    }

    if (nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC &&
        nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        error = "Unsupported PE optional header magic: " + path;
        return false;
    }

    size_of_headers_ = nt->OptionalHeader.SizeOfHeaders;

    const auto* section = IMAGE_FIRST_SECTION(nt);
    for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
        const auto& s = section[i];
        SectionInfo info;
        info.virtual_address = s.VirtualAddress;
        info.virtual_size = s.Misc.VirtualSize;
        info.raw_address = s.PointerToRawData;
        info.raw_size = s.SizeOfRawData;
        info.name = TrimSectionName(s);
        sections_.push_back(std::move(info));
    }

    return true;
}

bool PeImage::RvaToFileOffset(std::uint32_t rva, std::size_t& file_offset) const {
    if (rva < size_of_headers_) {
        if (rva < bytes_.size()) {
            file_offset = rva;
            return true;
        }
        return false;
    }

    for (const auto& section : sections_) {
        const std::uint32_t section_start = section.virtual_address;
        const std::uint32_t section_span = std::max(section.virtual_size, section.raw_size);
        const std::uint32_t section_end = section_start + section_span;
        if (rva < section_start || rva >= section_end) {
            continue;
        }

        const std::uint32_t delta = rva - section_start;
        if (delta >= section.raw_size) {
            return false;
        }

        const std::uint64_t off = static_cast<std::uint64_t>(section.raw_address) + delta;
        if (off >= bytes_.size()) {
            return false;
        }
        file_offset = static_cast<std::size_t>(off);
        return true;
    }
    return false;
}

bool PeImage::ReadBytesAtRva(std::uint32_t rva, std::size_t size, std::vector<std::uint8_t>& out) const {
    std::size_t file_offset = 0;
    if (!RvaToFileOffset(rva, file_offset)) {
        return false;
    }
    if (file_offset + size > bytes_.size()) {
        return false;
    }
    out.assign(bytes_.begin() + static_cast<std::ptrdiff_t>(file_offset),
               bytes_.begin() + static_cast<std::ptrdiff_t>(file_offset + size));
    return true;
}

bool PeImage::ReadWindowAtRva(std::uint32_t center_rva, std::size_t radius, std::uint32_t& window_start_rva, std::vector<std::uint8_t>& out) const {
    std::uint32_t start_rva = 0;
    std::uint32_t end_rva = 0;

    if (center_rva < size_of_headers_) {
        const std::uint32_t lower = (center_rva < radius) ? 0 : (center_rva - static_cast<std::uint32_t>(radius));
        const std::uint32_t upper_cap = (size_of_headers_ == 0) ? 0 : (size_of_headers_ - 1);
        const std::uint32_t upper_req = center_rva + static_cast<std::uint32_t>(radius);
        start_rva = lower;
        end_rva = std::min(upper_req, upper_cap);
    } else {
        bool found_section = false;
        for (const auto& section : sections_) {
            const std::uint32_t sec_start = section.virtual_address;
            const std::uint32_t sec_size = section.raw_size != 0 ? section.raw_size : section.virtual_size;
            if (sec_size == 0) {
                continue;
            }
            const std::uint32_t sec_end = sec_start + sec_size;
            if (center_rva < sec_start || center_rva >= sec_end) {
                continue;
            }

            const std::uint32_t lower_req = (center_rva < radius) ? 0 : (center_rva - static_cast<std::uint32_t>(radius));
            const std::uint32_t upper_req = center_rva + static_cast<std::uint32_t>(radius);
            start_rva = std::max(sec_start, lower_req);
            end_rva = std::min(sec_end - 1, upper_req);
            found_section = true;
            break;
        }
        if (!found_section) {
            return false;
        }
    }

    if (end_rva < start_rva) {
        return false;
    }

    window_start_rva = start_rva;
    const std::size_t window_size = static_cast<std::size_t>(end_rva - start_rva) + 1;
    out.clear();
    out.reserve(window_size);

    for (std::size_t i = 0; i < window_size; ++i) {
        const std::uint32_t rva = start_rva + static_cast<std::uint32_t>(i);
        std::size_t file_offset = 0;
        if (!RvaToFileOffset(rva, file_offset)) {
            return false;
        }
        out.push_back(bytes_[file_offset]);
    }
    return true;
}

bool PeImage::GetTextSection(std::uint32_t& text_rva, std::uint32_t& text_size) const {
    for (const auto& section : sections_) {
        if (section.name == ".text") {
            text_rva = section.virtual_address;
            text_size = section.raw_size != 0 ? section.raw_size : section.virtual_size;
            return true;
        }
    }
    return false;
}
