#include "embedded_resources.hpp"

#include <windows.h>

#include <cstdint>
#include <cstdio>

namespace EmbeddedResources {
namespace {

constexpr std::uint64_t kFnvOffset = 14695981039346656037ull;
constexpr std::uint64_t kFnvPrime = 1099511628211ull;

char LowerAscii(const char value) {
    if (value >= 'A' && value <= 'Z') {
        return static_cast<char>(value + ('a' - 'A'));
    }
    return value;
}

std::uint64_t HashPath(const std::string& path) {
    std::uint64_t hash = kFnvOffset;
    for (const unsigned char c : path) {
        hash ^= c;
        hash *= kFnvPrime;
    }
    return hash;
}

} // namespace

std::string NormalizePath(const std::string& path) {
    std::string normalized = path;
    for (char& c : normalized) {
        if (c == '\\') c = '/';
    }

    std::string lower = normalized;
    for (char& c : lower) {
        c = LowerAscii(c);
    }

    const char* roots[] = {
        "assetsanimated/",
        "assets/",
        "datamacro/"
    };

    for (const char* root : roots) {
        const std::size_t pos = lower.find(root);
        if (pos != std::string::npos) {
            normalized = normalized.substr(pos);
            lower = lower.substr(pos);
            break;
        }
    }

    while (lower.rfind("./", 0) == 0) {
        normalized.erase(0, 2);
        lower.erase(0, 2);
    }

    while (!lower.empty() && lower.front() == '/') {
        normalized.erase(0, 1);
        lower.erase(0, 1);
    }

    for (char& c : normalized) {
        c = LowerAscii(c);
    }
    return normalized;
}

std::string ResourceNameForPath(const std::string& path) {
    const std::string normalized = NormalizePath(path);
    const std::uint64_t hash = HashPath(normalized);

    char name[32] = {};
    std::snprintf(name, sizeof(name), "ASSET_%016llX",
        static_cast<unsigned long long>(hash));
    return name;
}

bool LoadBinary(const std::string& path, const unsigned char*& data, std::size_t& size) {
    data = nullptr;
    size = 0;

    const std::string resourceName = ResourceNameForPath(path);
    HRSRC resource = FindResourceA(nullptr, resourceName.c_str(), MAKEINTRESOURCEA(10));
    if (!resource) return false;

    HGLOBAL loaded = LoadResource(nullptr, resource);
    if (!loaded) return false;

    const DWORD resourceSize = SizeofResource(nullptr, resource);
    if (resourceSize == 0) return false;

    const void* resourceData = LockResource(loaded);
    if (!resourceData) return false;

    data = static_cast<const unsigned char*>(resourceData);
    size = static_cast<std::size_t>(resourceSize);
    return true;
}

bool LoadText(const std::string& path, std::string& out) {
    const unsigned char* data = nullptr;
    std::size_t size = 0;
    if (!LoadBinary(path, data, size)) return false;

    out.assign(reinterpret_cast<const char*>(data), size);
    return true;
}

} // namespace EmbeddedResources
