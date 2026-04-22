#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <Windows.h>

std::vector<int> ParsePattern(const char* pattern) {
    std::vector<int> bytes;
    char* ptr = const_cast<char*>(pattern);
    while (*ptr) {
        if (*ptr == '?') {
            bytes.push_back(-1);
            if (*(ptr+1) == '?') ptr++;
        } else if (isxdigit(*ptr)) {
            bytes.push_back(strtol(ptr, &ptr, 16));
            continue;
        }
        ptr++;
    }
    return bytes;
}

int main(int argc, char** argv) {
    if (argc < 3) return 1;
    std::ifstream fs(argv[1], std::ios::binary);
    if (!fs) return 1;
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
    
    auto pat = ParsePattern(argv[2]);
    std::cout << "[*] Scanning for pattern, size: " << pat.size() << "\n";
    
    int found = 0;
    for (size_t i = 0; i + pat.size() <= data.size(); ++i) {
        bool match = true;
        for (size_t j = 0; j < pat.size(); ++j) {
            if (pat[j] != -1 && data[i+j] != (uint8_t)pat[j]) {
                match = false; break;
            }
        }
        if (match) {
            std::cout << "[MATCH] Offset: 0x" << std::hex << i << " | Bytes: ";
            for (size_t j = 0; j < 16 && i+j < data.size(); ++j) 
                printf("%02X ", data[i+j]);
            std::cout << "\n";
            found++;
        }
    }
    std::cout << "[*] Total: " << found << "\n";
    return 0;
}
