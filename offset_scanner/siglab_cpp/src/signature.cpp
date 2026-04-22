#include "signature.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

namespace {
std::string UpperHexByte(std::uint8_t value) {
    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << static_cast<unsigned>(value);
    return oss.str();
}

bool IsRelJccOpcode(std::uint8_t b) {
    return b >= 0x80 && b <= 0x8F;
}

bool IsLikelyRipRelativePattern(const std::vector<std::uint8_t>& bytes, std::size_t i, bool with_rex) {
    if (with_rex) {
        if (i + 6 >= bytes.size()) {
            return false;
        }
        const std::uint8_t rex = bytes[i];
        if (rex < 0x40 || rex > 0x4F) {
            return false;
        }
        const std::uint8_t op = bytes[i + 1];
        if (!(op == 0x8B || op == 0x8D || op == 0x89 || op == 0x03 || op == 0x2B)) {
            return false;
        }
        const std::uint8_t modrm = bytes[i + 2];
        return (modrm & 0xC7) == 0x05;
    }

    if (i + 5 >= bytes.size()) {
        return false;
    }
    const std::uint8_t op = bytes[i];
    if (!(op == 0x8B || op == 0x8D || op == 0x89 || op == 0x03 || op == 0x2B)) {
        return false;
    }
    const std::uint8_t modrm = bytes[i + 1];
    return (modrm & 0xC7) == 0x05;
}

void WildcardRange(std::vector<SignatureToken>& pattern, std::size_t start, std::size_t count) {
    const std::size_t end = std::min(pattern.size(), start + count);
    for (std::size_t i = start; i < end; ++i) {
        pattern[i].value.reset();
    }
}
} // namespace

std::string BytesToHexPattern(const std::vector<std::uint8_t>& bytes) {
    std::ostringstream oss;
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        if (i != 0) {
            oss << ' ';
        }
        oss << UpperHexByte(bytes[i]);
    }
    return oss.str();
}

bool ParsePattern(const std::string& pattern, std::vector<SignatureToken>& out, std::string& error) {
    out.clear();
    std::istringstream iss(pattern);
    std::string token;
    while (iss >> token) {
        if (token == "?" || token == "??") {
            out.push_back(SignatureToken{std::nullopt});
            continue;
        }

        if (token.size() != 2 || !std::isxdigit(static_cast<unsigned char>(token[0])) || !std::isxdigit(static_cast<unsigned char>(token[1]))) {
            error = "Invalid token in pattern: " + token;
            return false;
        }

        const auto value = static_cast<std::uint8_t>(std::stoul(token, nullptr, 16));
        out.push_back(SignatureToken{value});
    }

    if (out.empty()) {
        error = "Pattern is empty";
        return false;
    }
    return true;
}

std::string TokensToPattern(const std::vector<SignatureToken>& tokens) {
    std::ostringstream oss;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        if (i != 0) {
            oss << ' ';
        }
        if (tokens[i].value.has_value()) {
            oss << UpperHexByte(tokens[i].value.value());
        } else {
            oss << "??";
        }
    }
    return oss.str();
}

std::vector<SignatureToken> BuildMaskedPattern(const std::vector<std::uint8_t>& bytes) {
    std::vector<SignatureToken> pattern;
    pattern.reserve(bytes.size());
    for (std::uint8_t b : bytes) {
        pattern.push_back(SignatureToken{b});
    }

    for (std::size_t i = 0; i < bytes.size(); ++i) {
        if (i + 4 < bytes.size() && (bytes[i] == 0xE8 || bytes[i] == 0xE9)) {
            WildcardRange(pattern, i + 1, 4);
        }
        if (i + 5 < bytes.size() && bytes[i] == 0x0F && IsRelJccOpcode(bytes[i + 1])) {
            WildcardRange(pattern, i + 2, 4);
        }
        if (IsLikelyRipRelativePattern(bytes, i, true)) {
            WildcardRange(pattern, i + 3, 4);
        }
        if (IsLikelyRipRelativePattern(bytes, i, false)) {
            WildcardRange(pattern, i + 2, 4);
        }
        if (i + 4 < bytes.size() && bytes[i] >= 0xB8 && bytes[i] <= 0xBF) {
            WildcardRange(pattern, i + 1, 4);
        }
    }

    return pattern;
}

MatchInfo ScanPatternInText(const PeImage& image, const std::vector<SignatureToken>& pattern, std::size_t capture_limit) {
    MatchInfo info;
    if (pattern.empty()) {
        return info;
    }

    std::uint32_t text_rva = 0;
    std::uint32_t text_size = 0;
    if (!image.GetTextSection(text_rva, text_size)) {
        return info;
    }

    std::vector<std::uint8_t> text;
    if (!image.ReadBytesAtRva(text_rva, text_size, text)) {
        return info;
    }

    if (text.size() < pattern.size()) {
        return info;
    }

    for (std::size_t i = 0; i <= text.size() - pattern.size(); ++i) {
        bool matched = true;
        for (std::size_t j = 0; j < pattern.size(); ++j) {
            if (pattern[j].value.has_value() && text[i + j] != pattern[j].value.value()) {
                matched = false;
                break;
            }
        }

        if (!matched) {
            continue;
        }

        ++info.match_count;
        if (info.first_match_rvas.size() < capture_limit) {
            info.first_match_rvas.push_back(text_rva + static_cast<std::uint32_t>(i));
        }
    }
    return info;
}
