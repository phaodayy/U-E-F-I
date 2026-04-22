#pragma once

#include "pe_image.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct SignatureToken {
    std::optional<std::uint8_t> value;
};

struct SignatureRule {
    std::string name;
    std::string module_name;
    std::uint32_t anchor_rva = 0;
    std::uint32_t window_start_rva = 0;
    std::string pattern_text;
    std::size_t expected_min = 1;
    std::size_t expected_max = 1;
};

struct MatchInfo {
    std::size_t match_count = 0;
    std::vector<std::uint32_t> first_match_rvas;
};

std::string BytesToHexPattern(const std::vector<std::uint8_t>& bytes);
bool ParsePattern(const std::string& pattern, std::vector<SignatureToken>& out, std::string& error);
std::string TokensToPattern(const std::vector<SignatureToken>& tokens);

std::vector<SignatureToken> BuildMaskedPattern(const std::vector<std::uint8_t>& bytes);
MatchInfo ScanPatternInText(const PeImage& image, const std::vector<SignatureToken>& pattern, std::size_t capture_limit);
