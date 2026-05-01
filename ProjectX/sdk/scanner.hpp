#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace Scanner {

inline bool ParsePattern(const char* pattern, std::vector<uint8_t>& bytes, std::string& mask) {
  bytes.clear();
  mask.clear();
  
  const char* p = pattern;
  while (*p) {
    if (*p == ' ') {
      p++;
      continue;
    }
    
    if (*p == '?') {
      bytes.push_back(0);
      mask.push_back('?');
      p++;
      if (*p == '?') p++;
    } else {
      char hex[3] = {p[0], p[1], 0};
      bytes.push_back((uint8_t)strtol(hex, nullptr, 16));
      mask.push_back('x');
      p += 2;
    }
  }
  
  return !bytes.empty();
}

inline uint64_t ScanBuffer(const uint8_t* buffer, size_t size, const std::vector<uint8_t>& pattern, const std::string& mask) {
  size_t patternLen = pattern.size();
  
  for (size_t i = 0; i < size - patternLen; i++) {
    bool found = true;
    
    for (size_t j = 0; j < patternLen; j++) {
      if (mask[j] == 'x' && buffer[i + j] != pattern[j]) {
        found = false;
        break;
      }
    }
    
    if (found)
      return i;
  }
  
  return 0;
}

template<typename ReadFunc>
inline uint64_t PatternScan(uint64_t start, size_t scanSize, const char* patternStr, ReadFunc ReadPhysical) {
  std::vector<uint8_t> pattern;
  std::string mask;
  
  if (!ParsePattern(patternStr, pattern, mask))
    return 0;
  
  constexpr size_t CHUNK_SIZE = 0x10000;
  std::vector<uint8_t> buffer(CHUNK_SIZE);
  
  for (size_t offset = 0; offset < scanSize; offset += CHUNK_SIZE) {
    size_t readSize = (offset + CHUNK_SIZE > scanSize) ? (scanSize - offset) : CHUNK_SIZE;
    
    if (!ReadPhysical(start + offset, buffer.data(), readSize))
      continue;
    
    uint64_t result = ScanBuffer(buffer.data(), readSize, pattern, mask);
    if (result)
      return start + offset + result;
  }
  
  return 0;
}

inline uint64_t ResolveRIPRelative(uint64_t instructionAddr, int32_t offset, int instructionLen) {
  return instructionAddr + instructionLen + offset;
}

}
