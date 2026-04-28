#pragma once
#include <cstdint>
#include <string>
#include <cstring>
#include <protec/skCrypt.h>



#pragma pack(push, 1)
struct FNameEntryHeader {
  uint16_t dwSTRLen : 15;
  uint16_t bWideSTR : 1;
};

struct FNameEntry {
  FNameEntryHeader NameEntryHeader;
  char first_char_encrypted[128];
};
#pragma pack(pop)

namespace FNames {

inline std::string DecryptName(const FNameEntry& Entry, uint32_t XorKey) {
  uint16_t dwStrLength = Entry.NameEntryHeader.dwSTRLen;
  bool bWideStr = Entry.NameEntryHeader.bWideSTR;
  
  if (bWideStr)
    dwStrLength = *(const uint16_t*)&Entry.NameEntryHeader & 0xFFFE;
  
  char chBuffer[1024] = {};
  if (dwStrLength >= 1024)
    return std::string();
  
  memcpy(chBuffer, Entry.first_char_encrypted, sizeof(Entry.first_char_encrypted));
  
  for (uint16_t i = 0; i < dwStrLength; i++) {
    chBuffer[i] ^= dwStrLength ^ *((uint8_t*)&XorKey + (i & 3));
  }
  
  chBuffer[dwStrLength] = '\0';
  return std::string(chBuffer);
}

template<typename ReadFunc>
inline uint32_t BruteForceFNameXorKey(uint64_t FNamePoolAddr, ReadFunc ReadMemory) {
  const std::string TargetFName = skCrypt("ByteProperty");
  const int FNameIndex = 3;
  
  int chunkOffset = FNameIndex >> 16;
  uint16_t nameOffset = FNameIndex & 0xFFFF;
  
  uint64_t FNamePoolChunk = ReadMemory(FNamePoolAddr + (chunkOffset + 2) * 8);
  if (!FNamePoolChunk)
    return 0;
  
  uint64_t FNameEntryOffset = FNamePoolChunk + (4ULL * nameOffset);
  FNameEntry fNameEntry = {};
  
  
  uint16_t dwStrLength = fNameEntry.NameEntryHeader.dwSTRLen;
  bool bWideStr = fNameEntry.NameEntryHeader.bWideSTR;
  
  if (bWideStr)
    dwStrLength = *(const uint16_t*)&fNameEntry.NameEntryHeader & 0xFFFE;
  
  uint8_t keyBytes[4] = {0};
  
  for (int byteIndex = 0; byteIndex < 4; byteIndex++) {
    for (int testByte = 0; testByte < 256; testByte++) {
      bool allMatch = true;
      
      for (uint16_t i = byteIndex; i < dwStrLength && i < TargetFName.length(); i += 4) {
        char decrypted = fNameEntry.first_char_encrypted[i] ^ dwStrLength ^ testByte;
        if (decrypted != TargetFName[i]) {
          allMatch = false;
          break;
        }
      }
      
      if (allMatch) {
        keyBytes[byteIndex] = (uint8_t)testByte;
        break;
      }
    }
  }
  
  uint32_t DecryptionKey = *((uint32_t*)keyBytes);
  
  std::string decryptedName = DecryptName(fNameEntry, DecryptionKey);
  
  if (decryptedName == TargetFName) {
    return DecryptionKey;
  }
  
  return 0;
}

template<typename ReadFunc>
inline std::string GetNameByIndex(uint64_t FNamePoolAddr, int32_t Index, uint32_t XorKey, ReadFunc ReadMemory) {
  if (Index < 0)
    return "";
  
  int chunkOffset = Index >> 16;
  uint16_t nameOffset = Index & 0xFFFF;
  
  uint64_t FNamePoolChunk = ReadMemory(FNamePoolAddr + (chunkOffset + 2) * 8);
  if (!FNamePoolChunk)
    return "";
  
  uint64_t FNameEntryOffset = FNamePoolChunk + (4ULL * nameOffset);
  FNameEntry fNameEntry = {};
  
  
  return DecryptName(fNameEntry, XorKey);
}

} 

namespace FNameUtils {
uint32_t GetKey();
std::string GetNameFast(int32_t index);
bool IsA(uint64_t actor, const std::string &class_name);
} 
