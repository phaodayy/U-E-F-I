#pragma once

#include <cstddef>
#include <string>

namespace EmbeddedResources {

std::string NormalizePath(const std::string& path);
std::string ResourceNameForPath(const std::string& path);
bool LoadBinary(const std::string& path, const unsigned char*& data, std::size_t& size);
bool LoadText(const std::string& path, std::string& out);

} // namespace EmbeddedResources
