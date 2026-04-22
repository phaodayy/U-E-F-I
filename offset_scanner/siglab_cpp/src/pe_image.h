#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class PeImage {
public:
    bool LoadFromFile(const std::string& path, std::string& error);

    [[nodiscard]] const std::string& path() const { return path_; }
    [[nodiscard]] const std::vector<std::uint8_t>& bytes() const { return bytes_; }

    bool RvaToFileOffset(std::uint32_t rva, std::size_t& file_offset) const;
    bool ReadBytesAtRva(std::uint32_t rva, std::size_t size, std::vector<std::uint8_t>& out) const;
    bool ReadWindowAtRva(std::uint32_t center_rva, std::size_t radius, std::uint32_t& window_start_rva, std::vector<std::uint8_t>& out) const;
    bool GetTextSection(std::uint32_t& text_rva, std::uint32_t& text_size) const;

private:
    struct SectionInfo {
        std::uint32_t virtual_address = 0;
        std::uint32_t virtual_size = 0;
        std::uint32_t raw_address = 0;
        std::uint32_t raw_size = 0;
        std::string name;
    };

    std::string path_;
    std::vector<std::uint8_t> bytes_;
    std::vector<SectionInfo> sections_;
    std::uint32_t size_of_headers_ = 0;
};
