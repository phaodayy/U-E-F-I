#include "pe_image.h"
#include "signature.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {
struct CliArgs {
    std::vector<std::string> positional;
    std::unordered_map<std::string, std::vector<std::string>> options;
};

CliArgs ParseArgs(int argc, char** argv) {
    CliArgs args;
    for (int i = 1; i < argc; ++i) {
        std::string token = argv[i];
        if (token.rfind("--", 0) == 0) {
            std::string key = token.substr(2);
            std::string value = "true";
            if (i + 1 < argc) {
                std::string next = argv[i + 1];
                if (next.rfind("--", 0) != 0) {
                    value = next;
                    ++i;
                }
            }
            args.options[key].push_back(value);
        } else {
            args.positional.push_back(std::move(token));
        }
    }
    return args;
}

std::optional<std::string> GetOption(const CliArgs& args, const std::string& key) {
    const auto it = args.options.find(key);
    if (it == args.options.end() || it->second.empty()) {
        return std::nullopt;
    }
    return it->second.front();
}

std::vector<std::string> GetOptions(const CliArgs& args, const std::string& key) {
    const auto it = args.options.find(key);
    if (it == args.options.end()) {
        return {};
    }
    return it->second;
}

bool ParseU32(const std::string& text, std::uint32_t& out) {
    try {
        std::size_t idx = 0;
        unsigned long long value = 0;
        if (text.rfind("0x", 0) == 0 || text.rfind("0X", 0) == 0) {
            value = std::stoull(text, &idx, 16);
        } else {
            value = std::stoull(text, &idx, 10);
        }
        if (idx != text.size() || value > std::numeric_limits<std::uint32_t>::max()) {
            return false;
        }
        out = static_cast<std::uint32_t>(value);
        return true;
    } catch (...) {
        return false;
    }
}

std::string HexU32(std::uint32_t value) {
    std::ostringstream oss;
    oss << "0x" << std::uppercase << std::hex << value;
    return oss.str();
}

bool ReadJsonFile(const fs::path& path, json& out, std::string& error) {
    std::ifstream ifs(path);
    if (!ifs) {
        error = "Failed to open JSON file: " + path.string();
        return false;
    }
    try {
        ifs >> out;
        return true;
    } catch (const std::exception& ex) {
        error = "Failed to parse JSON " + path.string() + ": " + ex.what();
        return false;
    }
}

bool WriteJsonFile(const fs::path& path, const json& doc, std::string& error) {
    std::ofstream ofs(path);
    if (!ofs) {
        error = "Failed to write JSON file: " + path.string();
        return false;
    }
    ofs << std::setw(2) << doc << "\n";
    return true;
}

bool JsonReadU32(const json& j, const std::string& key, std::uint32_t& out) {
    if (!j.contains(key)) {
        return false;
    }
    if (j[key].is_number_unsigned()) {
        out = j[key].get<std::uint32_t>();
        return true;
    }
    if (j[key].is_string()) {
        return ParseU32(j[key].get<std::string>(), out);
    }
    return false;
}

bool LoadRuleFromJson(const json& j, SignatureRule& rule, std::string& error) {
    if (!j.contains("name") || !j.contains("module_name") || !j.contains("pattern")) {
        error = "Rule JSON must contain: name, module_name, pattern";
        return false;
    }

    rule.name = j["name"].get<std::string>();
    rule.module_name = j["module_name"].get<std::string>();
    rule.pattern_text = j["pattern"].get<std::string>();

    if (!JsonReadU32(j, "anchor_rva", rule.anchor_rva)) {
        rule.anchor_rva = 0;
    }
    if (!JsonReadU32(j, "window_start_rva", rule.window_start_rva)) {
        rule.window_start_rva = 0;
    }

    if (j.contains("expected_min") && j["expected_min"].is_number_unsigned()) {
        rule.expected_min = j["expected_min"].get<std::size_t>();
    }
    if (j.contains("expected_max") && j["expected_max"].is_number_unsigned()) {
        rule.expected_max = j["expected_max"].get<std::size_t>();
    }
    if (rule.expected_min > rule.expected_max) {
        error = "Rule expected_min is greater than expected_max";
        return false;
    }
    return true;
}

int RunValidateRuleOnModule(const SignatureRule& rule, const std::string& module_path) {
    std::vector<SignatureToken> tokens;
    std::string parse_error;
    if (!ParsePattern(rule.pattern_text, tokens, parse_error)) {
        std::cerr << "[-] Pattern parse error in rule '" << rule.name << "': " << parse_error << "\n";
        return 2;
    }

    PeImage image;
    std::string image_error;
    if (!image.LoadFromFile(module_path, image_error)) {
        std::cerr << "[-] " << image_error << "\n";
        return 2;
    }

    MatchInfo info = ScanPatternInText(image, tokens, 5);
    const bool pass = info.match_count >= rule.expected_min && info.match_count <= rule.expected_max;

    std::cout << "[RULE] " << rule.name << " | Module: " << module_path << "\n";
    std::cout << "       matches=" << info.match_count
              << " expected=[" << rule.expected_min << ", " << rule.expected_max << "]"
              << " => " << (pass ? "PASS" : "FAIL") << "\n";
    if (!info.first_match_rvas.empty()) {
        std::cout << "       first_rvas: ";
        for (std::size_t i = 0; i < info.first_match_rvas.size(); ++i) {
            if (i != 0) {
                std::cout << ", ";
            }
            std::cout << HexU32(info.first_match_rvas[i]);
        }
        std::cout << "\n";
    }
    return pass ? 0 : 1;
}

int CommandFindRefs(const CliArgs& args) {
    const auto module = GetOption(args, "module");
    const auto target = GetOption(args, "target-rva");
    if (!module || !target) {
        std::cerr << "Usage: siglab find-refs --module <path> --target-rva <hex>\n";
        return 2;
    }

    std::uint32_t target_rva = 0;
    if (!ParseU32(*target, target_rva)) {
        std::cerr << "[-] Invalid --target-rva: " << *target << "\n";
        return 2;
    }

    PeImage image;
    std::string error;
    if (!image.LoadFromFile(*module, error)) {
        std::cerr << "[-] " << error << "\n";
        return 2;
    }

    std::uint32_t text_rva = 0;
    std::uint32_t text_size = 0;
    if (!image.GetTextSection(text_rva, text_size)) {
        std::cerr << "[-] Failed to find .text section\n";
        return 2;
    }

    std::vector<std::uint8_t> text;
    if (!image.ReadBytesAtRva(text_rva, text_size, text)) {
        std::cerr << "[-] Failed to read .text section bytes\n";
        return 2;
    }

    std::cout << "[*] Scanning for references to " << HexU32(target_rva) << " in .text...\n";

    std::size_t found_count = 0;
    for (std::size_t i = 0; i + 7 <= text.size(); ++i) {
        bool is_rip_rel = false;
        std::int32_t disp = 0;
        std::uint32_t rip_at = text_rva + (std::uint32_t)i;
        
        // Match 48/4C 8B/8D/89/3B/39/03/2B 05 (64-bit RIP-rel)
        if ((text[i] == 0x48 || text[i] == 0x4C) && 
            (text[i+1] == 0x8B || text[i+1] == 0x8D || text[i+1] == 0x89 || 
             text[i+1] == 0x3B || text[i+1] == 0x39 || text[i+1] == 0x03 || text[i+1] == 0x2B) && 
            text[i+2] == 0x05) 
        {
            is_rip_rel = true;
            std::memcpy(&disp, &text[i+3], 4);
        }
        // Match 8B/8D/89/3B/39/03/2B 05 (32-bit RIP-rel/Legacy)
        else if ((text[i] == 0x8B || text[i] == 0x8D || text[i] == 0x89 || 
                  text[i] == 0x3B || text[i] == 0x39 || text[i] == 0x03 || text[i] == 0x2B) && 
                 text[i+1] == 0x05)
        {
            is_rip_rel = true;
            std::memcpy(&disp, &text[i+2], 4);
            rip_at++; // Base is +1 because of no REX
        }
        
        if (is_rip_rel) {
            std::uint32_t absolute = rip_at + 6 + disp; // + 6 total if no REX, + 7 if REX. 
            // Wait, logic for absolute calculation depends on instruction length.
            // Simplified: let's just check both +6 and +7.
            if (absolute == target_rva || (rip_at + 7 + disp) == target_rva) {
                std::cout << "[+] Found reference at " << HexU32(rip_at) << ": " 
                          << BytesToHexPattern({text.begin()+i, text.begin()+i+7}) << std::endl;
                found_count++;
            }
        }
    }

    std::cout << "[*] Total references found: " << found_count << "\n";
    return 0;
}

int CommandBulkGenerate(const CliArgs& args) {
    const auto config_path = GetOption(args, "config");
    const auto module_path = GetOption(args, "module");
    const auto rules_dir = GetOption(args, "rules-out").value_or("rules");
    if (!config_path || !module_path) {
        std::cerr << "Usage: siglab bulk-generate --config <pubg_config.hpp> --module <dump.exe> [--rules-out <dir>]\n";
        return 2;
    }

    std::ifstream ifs(*config_path);
    if (!ifs) {
        std::cerr << "[-] Failed to open config file: " << *config_path << "\n";
        return 2;
    }

    fs::create_directories(rules_dir);

    PeImage image;
    std::string error;
    if (!image.LoadFromFile(*module_path, error)) {
        std::cerr << "[-] " << error << "\n";
        return 2;
    }

    std::uint32_t text_rva = 0;
    std::uint32_t text_size = 0;
    if (!image.GetTextSection(text_rva, text_size)) {
        std::cerr << "[-] Failed to find .text section\n";
        return 2;
    }

    std::vector<std::uint8_t> text;
    if (!image.ReadBytesAtRva(text_rva, text_size, text)) {
        std::cerr << "[-] Failed to read .text section bytes\n";
        return 2;
    }

    std::cout << "[*] Parsing config file: " << *config_path << "\n";

    std::string line;
    std::size_t processed = 0;
    std::size_t generated = 0;

    while (std::getline(ifs, line)) {
        // Skip comments and empty lines
        if (line.find("//") != std::string::npos) {
            line = line.substr(0, line.find("//"));
        }

        // Very basic simple regex-like search for "inline uint64_t Name = 0xValue;"
        if (line.find("inline uint64_t") == std::string::npos || line.find("=") == std::string::npos) continue;

        size_t start = line.find("uint64_t") + 8;
        while (start < line.size() && isspace(line[start])) start++;
        size_t end = line.find("=", start);
        if (end == std::string::npos) continue;
        
        std::string name = line.substr(start, end - start);
        name.erase(0, name.find_first_not_of(" \t\n\r\f\v"));
        name.erase(name.find_last_not_of(" \t\n\r\f\v") + 1);

        size_t v_start = end + 1;
        while (v_start < line.size() && (isspace(line[v_start]) || line[v_start] == '=')) v_start++;
        size_t v_end = line.find(";", v_start);
        if (v_end == std::string::npos) v_end = line.size();

        std::string v_text = line.substr(v_start, v_end - v_start);
        v_text.erase(0, v_text.find_first_not_of(" \t\n\r\f\v"));
        v_text.erase(v_text.find_last_not_of(" \t\n\r\f\v") + 1);

        std::uint32_t target_rva = 0;
        if (!ParseU32(v_text, target_rva)) continue;

        if (target_rva == 0) continue; 

        processed++;
        std::uint32_t found_ref_rva = 0;

        // Find reference with improved opcode support
        for (std::size_t i = 0; i + 7 <= text.size(); ++i) {
             bool match = false;
             std::int32_t d = 0;
             std::uint32_t inst_len = 0;

             if ((text[i] == 0x48 || text[i] == 0x4C) && 
                 (text[i+1] == 0x8B || text[i+1] == 0x8D || text[i+1] == 0x89 || 
                  text[i+1] == 0x3B || text[i+1] == 0x39 || text[i+1] == 0x03 || text[i+1] == 0x2B) && 
                 text[i+2] == 0x05) 
             {
                match = true;
                std::memcpy(&d, &text[i+3], 4);
                inst_len = 7;
             }
             else if ((text[i] == 0x8B || text[i] == 0x8D || text[i] == 0x89 || 
                       text[i] == 0x3B || text[i] == 0x39 || text[i] == 0x03 || text[i] == 0x2B) && 
                      text[i+1] == 0x05)
             {
                match = true;
                std::memcpy(&d, &text[i+2], 4);
                inst_len = 6;
             }

             if (match && (text_rva + (std::uint32_t)i + inst_len + d == target_rva)) {
                found_ref_rva = text_rva + (std::uint32_t)i;
                break; 
             }
        }

        // 2. Try Immediate Constants (Keys/Offsets)
        if (found_ref_rva == 0 && target_rva > 0xFFFF) {
            for (std::size_t i = 0; i + 4 <= text.size(); ++i) {
                std::uint32_t val = 0;
                std::memcpy(&val, &text[i], 4);
                if (val == target_rva) {
                    if (i > 0) {
                        std::uint8_t op = text[i-1];
                        if (op == 0xB8 || op == 0xBA || op == 0xB9 || op == 0xBF || op == 0xBE || op == 0xBD || op == 0xBC) {
                            found_ref_rva = text_rva + (std::uint32_t)(i - 1);
                            break;
                        }
                    }
                }
            }
        }

        if (found_ref_rva != 0) {
            std::uint32_t window_start = 0;
            std::vector<std::uint8_t> window_bytes;
            if (image.ReadWindowAtRva(found_ref_rva, 16, window_start, window_bytes)) {
                const std::vector<SignatureToken> masked = BuildMaskedPattern(window_bytes);
                
                json rule;
                rule["name"] = name;
                rule["module_name"] = "TslGame.exe";
                rule["anchor_rva"] = HexU32(found_ref_rva);
                rule["window_start_rva"] = HexU32(window_start);
                rule["pattern"] = TokensToPattern(masked);
                rule["expected_min"] = 1;
                rule["expected_max"] = 1;
                rule["generator"] = "siglab_cpp bulk-generate";

                fs::path out_path = fs::path(rules_dir) / (name + ".json");
                std::string write_err;
                if (WriteJsonFile(out_path, rule, write_err)) {
                    generated++;
                    std::cout << "[+] Rule for '" << name << "' generated at " << HexU32(found_ref_rva) << std::endl;
                }
            }
        }
    }

    std::cout << "[SUMMARY] Processed " << processed << " offsets from config. Generated " << generated << " rules." << std::endl;

    std::cout << "[SUMMARY] Processed " << processed << " offsets from config. Generated " << generated << " rules.\n";
    return 0;
}

int CommandExtract(const CliArgs& args) {
    const auto module = GetOption(args, "module");
    const auto anchor = GetOption(args, "anchor-rva");
    const auto out = GetOption(args, "out");
    if (!module || !anchor || !out) {
        std::cerr << "Usage: siglab extract --module <path> --anchor-rva <hex> --out <file> [--radius <n>]\n";
        return 2;
    }

    std::uint32_t anchor_rva = 0;
    if (!ParseU32(*anchor, anchor_rva)) {
        std::cerr << "[-] Invalid --anchor-rva: " << *anchor << "\n";
        return 2;
    }

    std::size_t radius = 48;
    if (const auto radius_text = GetOption(args, "radius")) {
        std::uint32_t parsed = 0;
        if (!ParseU32(*radius_text, parsed)) {
            std::cerr << "[-] Invalid --radius: " << *radius_text << "\n";
            return 2;
        }
        radius = parsed;
    }

    PeImage image;
    std::string error;
    if (!image.LoadFromFile(*module, error)) {
        std::cerr << "[-] " << error << "\n";
        return 2;
    }

    std::uint32_t window_start = 0;
    std::vector<std::uint8_t> window_bytes;
    if (!image.ReadWindowAtRva(anchor_rva, radius, window_start, window_bytes)) {
        std::cerr << "[-] Failed to read window around RVA " << HexU32(anchor_rva) << "\n";
        return 2;
    }

    json j;
    j["version"] = 1;
    j["module_path"] = fs::path(*module).string();
    j["module_name"] = fs::path(*module).filename().string();
    j["anchor_rva"] = HexU32(anchor_rva);
    j["window_start_rva"] = HexU32(window_start);
    j["window_size"] = window_bytes.size();
    j["radius"] = radius;
    j["bytes"] = BytesToHexPattern(window_bytes);

    if (!WriteJsonFile(*out, j, error)) {
        std::cerr << "[-] " << error << "\n";
        return 2;
    }

    std::cout << "[+] Extracted window bytes\n";
    std::cout << "    module: " << *module << "\n";
    std::cout << "    anchor_rva: " << HexU32(anchor_rva) << "\n";
    std::cout << "    window_start_rva: " << HexU32(window_start) << "\n";
    std::cout << "    size: " << window_bytes.size() << " bytes\n";
    std::cout << "    out: " << *out << "\n";
    return 0;
}

int CommandMask(const CliArgs& args) {
    const auto in = GetOption(args, "in");
    const auto out = GetOption(args, "out");
    if (!in || !out) {
        std::cerr << "Usage: siglab mask --in <extract.json> --out <rule.json> [--name <rule_name>] [--module-name <name>]\n";
        return 2;
    }

    json input;
    std::string error;
    if (!ReadJsonFile(*in, input, error)) {
        std::cerr << "[-] " << error << "\n";
        return 2;
    }

    if (!input.contains("bytes") || !input["bytes"].is_string()) {
        std::cerr << "[-] Input extract JSON missing string field: bytes\n";
        return 2;
    }

    std::vector<SignatureToken> raw_tokens;
    std::string parse_error;
    if (!ParsePattern(input["bytes"].get<std::string>(), raw_tokens, parse_error)) {
        std::cerr << "[-] Invalid bytes pattern in extract JSON: " << parse_error << "\n";
        return 2;
    }

    std::vector<std::uint8_t> raw_bytes;
    raw_bytes.reserve(raw_tokens.size());
    for (const auto& token : raw_tokens) {
        if (!token.value.has_value()) {
            std::cerr << "[-] Extract bytes must not contain wildcards.\n";
            return 2;
        }
        raw_bytes.push_back(token.value.value());
    }

    const std::vector<SignatureToken> masked = BuildMaskedPattern(raw_bytes);
    const std::string pattern_text = TokensToPattern(masked);

    const std::string module_name =
        GetOption(args, "module-name").value_or(input.value("module_name", std::string("TslGame.exe")));
    const std::string rule_name =
        GetOption(args, "name").value_or(std::string("auto_rule_") + module_name);

    std::uint32_t anchor_rva = 0;
    std::uint32_t window_start_rva = 0;
    JsonReadU32(input, "anchor_rva", anchor_rva);
    JsonReadU32(input, "window_start_rva", window_start_rva);

    json rule;
    rule["name"] = rule_name;
    rule["module_name"] = module_name;
    rule["anchor_rva"] = HexU32(anchor_rva);
    rule["window_start_rva"] = HexU32(window_start_rva);
    rule["pattern"] = pattern_text;
    rule["expected_min"] = 1;
    rule["expected_max"] = 1;
    rule["generator"] = "siglab_cpp mask";

    if (!WriteJsonFile(*out, rule, error)) {
        std::cerr << "[-] " << error << "\n";
        return 2;
    }

    std::cout << "[+] Generated masked signature rule\n";
    std::cout << "    rule: " << rule_name << "\n";
    std::cout << "    module_name: " << module_name << "\n";
    std::cout << "    pattern_len: " << masked.size() << " tokens\n";
    std::cout << "    out: " << *out << "\n";
    return 0;
}

int CommandValidate(const CliArgs& args) {
    const auto rule_path = GetOption(args, "rule");
    const auto modules = GetOptions(args, "module");
    if (!rule_path || modules.empty()) {
        std::cerr << "Usage: siglab validate --rule <rule.json> --module <file> [--module <file> ...]\n";
        return 2;
    }

    json doc;
    std::string error;
    if (!ReadJsonFile(*rule_path, doc, error)) {
        std::cerr << "[-] " << error << "\n";
        return 2;
    }

    SignatureRule rule;
    if (!LoadRuleFromJson(doc, rule, error)) {
        std::cerr << "[-] " << error << "\n";
        return 2;
    }

    int worst = 0;
    for (const auto& module : modules) {
        worst = std::max(worst, RunValidateRuleOnModule(rule, module));
    }
    return worst;
}

int CommandScan(const CliArgs& args) {
    const auto rules_dir = GetOption(args, "rules");
    const auto module = GetOption(args, "module");
    if (!rules_dir || !module) {
        std::cerr << "Usage: siglab scan --rules <dir> --module <file>\n";
        return 2;
    }

    std::size_t total = 0;
    std::size_t passed = 0;

    for (const auto& entry : fs::directory_iterator(*rules_dir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }

        json doc;
        std::string error;
        if (!ReadJsonFile(entry.path(), doc, error)) {
            std::cerr << "[-] " << error << "\n";
            continue;
        }

        SignatureRule rule;
        if (!LoadRuleFromJson(doc, rule, error)) {
            std::cerr << "[-] " << entry.path().string() << ": " << error << "\n";
            continue;
        }

        ++total;
        const int rc = RunValidateRuleOnModule(rule, *module);
        if (rc == 0) {
            ++passed;
        }
    }

    std::cout << "[SUMMARY] pass=" << passed << "/" << total << "\n";
    return (total > 0 && passed == total) ? 0 : 1;
}

void PrintUsage() {
    std::cout << "siglab_cpp - Offset-to-signature scaffold\n\n";
    std::cout << "Commands:\n";
    std::cout << "  extract   --module <path> --anchor-rva <hex> --out <extract.json> [--radius <n>]\n";
    std::cout << "  mask      --in <extract.json> --out <rule.json> [--name <rule_name>] [--module-name <name>]\n";
    std::cout << "  validate  --rule <rule.json> --module <file> [--module <file> ...]\n";
    std::cout << "  scan          --rules <dir> --module <file>\n";
    std::cout << "  find-refs     --module <path> --target-rva <hex>\n";
    std::cout << "  bulk-generate --config <file> --module <file> [--rules-out <dir>]\n";
    std::cout << "  scan-config   --config <file> --module <file>\n";
}
} // namespace

int CommandFindImm(const CliArgs& args) {
    const auto module_path = GetOption(args, "module");
    const auto value_str = GetOption(args, "value");
    if (!module_path || !value_str) {
        std::cerr << "Usage: siglab find-imm --module <dump.exe> --value <hex_offset>\n";
        return 2;
    }

    uint32_t target_val = 0;
    if (!ParseU32(*value_str, target_val)) return 2;

    PeImage image;
    std::string error;
    if (!image.LoadFromFile(*module_path, error)) {
        std::cerr << "[-] " << error << "\n";
        return 2;
    }

    uint32_t text_rva = 0, text_size = 0;
    image.GetTextSection(text_rva, text_size);
    std::vector<uint8_t> text;
    image.ReadBytesAtRva(text_rva, text_size, text);

    std::cout << "[*] Searching for immediate value 0x" << std::hex << target_val << " in instructions...\n";

    struct Candidate { uint32_t rva; std::string pattern; int matches; };
    std::vector<Candidate> candidates;

    for (size_t i = 0; i + 4 <= text.size(); ++i) {
        uint32_t val;
        std::memcpy(&val, &text[i], 4);
        
        if (val == target_val) {
            // Check if it's an offset in a MOV/ADD/LEA instruction
            // Heuristic: check if previous byte is a ModR/M or Opcode
            if (i >= 2) {
                uint32_t anchor_rva = text_rva + (uint32_t)i - 2;
                uint32_t window_start = 0;
                std::vector<uint8_t> window_bytes;
                if (image.ReadWindowAtRva(anchor_rva, 10, window_start, window_bytes)) {
                    auto masked = BuildMaskedPattern(window_bytes);
                    std::string pat = TokensToPattern(masked);
                    
                    // Simple uniqueness check
                    int matches = 0;
                    std::vector<SignatureToken> pat_tokens;
                    std::string p_err;
                    if (ParsePattern(pat, pat_tokens, p_err)) {
                        for (size_t k = 0; k + pat_tokens.size() <= text.size(); ++k) {
                            bool match = true;
                            for (size_t j = 0; j < pat_tokens.size(); ++j) {
                                if (pat_tokens[j].value.has_value() && text[k+j] != pat_tokens[j].value.value()) {
                                    match = false; break;
                                }
                            }
                            if (match) matches++;
                            if (matches > 10) break; 
                        }
                    }

                    if (matches > 0) {
                        candidates.push_back({anchor_rva, pat, matches});
                    }
                }
            }
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        return a.matches < b.matches;
    });

    std::cout << "[DEBUG] Found " << candidates.size() << " potential signatures.\n\n";
    std::cout << "Top Unique Signatures:\n";
    for (int i = 0; i < std::min((int)candidates.size(), 5); ++i) {
        std::cout << "[" << (i+1) << "] Matches: " << candidates[i].matches << " | RVA: " << HexU32(candidates[i].rva) << "\n";
        std::cout << "    Pattern: " << candidates[i].pattern << "\n\n";
    }

    return 0;
}

int CommandExtractOffset(const CliArgs& args) {
    const auto module_path = GetOption(args, "module");
    const auto sig_str = GetOption(args, "sig");
    const auto skip_str = GetOption(args, "skip");
    if (!module_path || !sig_str) {
        std::cerr << "Usage: siglab extract-offset --module <dump.exe> --sig \"...\" [--skip 10]\n";
        return 2;
    }

    int skip_bytes = 0;
    if (skip_str) skip_bytes = std::stoi(*skip_str);

    PeImage image;
    std::string error;
    if (!image.LoadFromFile(*module_path, error)) {
        std::cerr << "[-] " << error << "\n";
        return 2;
    }

    uint32_t text_rva = 0, text_size = 0;
    image.GetTextSection(text_rva, text_size);
    std::vector<uint8_t> text;
    image.ReadBytesAtRva(text_rva, text_size, text);

    std::vector<SignatureToken> tokens;
    if (!ParsePattern(*sig_str, tokens, error)) {
        std::cerr << "[-] Invalid pattern: " << error << "\n";
        return 2;
    }

    std::cout << "[*] Scanning for " << *sig_str << "...\n";
    auto match = ScanPatternInText(image, tokens, 5);
    if (match.match_count == 0) {
        std::cerr << "[-] No matches found.\n";
        return 1;
    }

    for (size_t i = 0; i < match.first_match_rvas.size(); ++i) {
        uint32_t rva = match.first_match_rvas[i];
        uint32_t offset_in_text = (rva - text_rva) + skip_bytes;
        uint8_t opcode = text[offset_in_text];
        uint8_t rex = 0;
        
        if (opcode >= 0x40 && opcode <= 0x4F) { // REX prefix
            rex = opcode;
            opcode = text[++offset_in_text];
        }

        std::cout << "\n[Match " << (i+1) << "] RVA: 0x" << std::hex << rva << "\n";
        
        // --- Logic inspired by the 'Fishing' lesson ---
        
        // Case 1: RIP-Relative (MOV/LEA [RIP + disp32])
        // Pattern: 48 8B 05 (mov rax, [rip+...]) or 48 8D 0D (lea rcx, [rip+...]) or 48 89 05 (mov [rip+...], rax)
        if (opcode == 0x8B || opcode == 0x8D || opcode == 0x89) {
            uint8_t modrm = text[offset_in_text + 1];
            if ((modrm & 0xC7) == 0x05) { // [RIP + disp32]
                int32_t disp;
                std::memcpy(&disp, &text[offset_in_text + 2], 4);
                uint32_t target_rva = rva + (offset_in_text - (rva - text_rva)) + 6 + disp;
                std::cout << "  TYPE: RIP-Relative Global Pointer\n";
                std::cout << "  VAL : 0x" << std::hex << target_rva << " (Absolute RVA)\n";
                continue;
            }
        }

        // Case 2: Member Offset (MOV reg, [reg + disp32])
        // Pattern: 48 8B 80 / 48 8B 88 etc.
        if (opcode == 0x8B || opcode == 0x89) {
            uint8_t modrm = text[offset_in_text + 1];
            if ((modrm & 0xC0) == 0x80) { // [reg + disp32]
                int32_t disp;
                std::memcpy(&disp, &text[offset_in_text + 2], 4);
                std::cout << "  TYPE: Member Offset (32-bit Displacement)\n";
                std::cout << "  VAL : 0x" << std::hex << disp << "\n";
                continue;
            }
            if ((modrm & 0xC0) == 0x40) { // [reg + disp8]
                int8_t disp = (int8_t)text[offset_in_text + 2];
                std::cout << "  TYPE: Member Offset (8-bit Displacement)\n";
                std::cout << "  VAL : 0x" << std::hex << (int32_t)disp << "\n";
                continue;
            }
        }

        // Case 3: Immediate (MOV reg, imm32)
        // Pattern: B8 / B9 / BA / BB / BF ...
        if (opcode >= 0xB8 && opcode <= 0xBF) {
            uint32_t imm;
            std::memcpy(&imm, &text[offset_in_text + 1], 4);
            std::cout << "  TYPE: Immediate Constant (32-bit)\n";
            std::cout << "  VAL : 0x" << std::hex << imm << "\n";
            continue;
        }

        // Case 4: Small Immediate (imm8)
        // Many opcodes have imm8 versions. This is a heuristic.
        if (opcode == 0xC1 || opcode == 0x83 || opcode == 0x6A) {
            uint8_t imm = text[offset_in_text + 2]; // Usually follows ModRM
            std::cout << "  TYPE: Immediate Constant (8-bit)\n";
            std::cout << "  VAL : 0x" << std::hex << (int)imm << "\n";
            continue;
        }

        std::cout << "  TYPE: Unknown Opcode (0x" << std::hex << (int)opcode << "). Manual analysis required.\n";
    }

    return 0;
}

int CommandMakeSig(const CliArgs& args) {
    const auto module_path = GetOption(args, "module");
    const auto rva_str = GetOption(args, "rva");
    if (!module_path || !rva_str) {
        std::cerr << "Usage: siglab make-sig --module <dump.exe> --rva <hex>\n";
        return 2;
    }

    uint32_t target_rva = 0;
    if (!ParseU32(*rva_str, target_rva)) return 2;

    PeImage image;
    std::string error;
    if (!image.LoadFromFile(*module_path, error)) {
        std::cerr << "[-] " << error << "\n";
        return 2;
    }

    uint32_t window_start = 0;
    std::vector<uint8_t> window_bytes;
    if (!image.ReadWindowAtRva(target_rva, 24, window_start, window_bytes)) {
        std::cerr << "[-] Failed to read code at 0x" << std::hex << target_rva << "\n";
        return 2;
    }

    std::cout << "[*] Generating Signature Candidates for instruction at 0x" << std::hex << target_rva << "...\n";
    
    // Candidate 1: Smart Mask (Standard pointers masked)
    auto masked = BuildMaskedPattern(window_bytes);
    std::string sig_smart = TokensToPattern(masked);
    
    // Candidate 2: Strict Opcode (Mask all but first byte of each instruction?) 
    // Since we don't have a full decoder here, we'll suggest variations.
    
    std::cout << "\n[Candidate 1: Smart Mask (Stable)]\n";
    std::cout << "  " << sig_smart << "\n";
    
    // Candidate 3: Value/Pointer agnostic (Mask all 4-byte sequences that look like RVA/Disp)
    std::vector<uint8_t> agnostic_bytes = window_bytes;
    std::string sig_agnostic = "";
    auto toHex = [](uint8_t b) {
        char buf[4];
        sprintf_s(buf, "%02X ", b);
        return std::string(buf);
    };

    for (size_t i = 0; i < agnostic_bytes.size(); ++i) {
        // Heuristic: Mask displacements that follow common opcodes
        bool should_mask = false;
        if (i >= 2 && (agnostic_bytes[i-2] == 0x8B || agnostic_bytes[i-2] == 0x8D)) should_mask = true; // MOV/LEA
        
        if (should_mask && i + 4 <= agnostic_bytes.size()) {
            sig_agnostic += "?? ?? ?? ?? ";
            i += 3;
        } else {
            sig_agnostic += toHex(agnostic_bytes[i]);
        }
    }
    std::cout << "\n[Candidate 2: Offset Agnostic (Very Stable)]\n";
    std::cout << "  " << sig_agnostic << "\n";

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "[*] No arguments provided. Attempting AUTO-SCAN...\n";
        
        // Default paths based on project structure
        std::string default_config = "../../PUBG-2/.shared/pubg_config.hpp";
        std::string default_module = "../Process-Dumper/bin/dump_TslGame.exe";
        
        if (!fs::exists(default_config)) {
            // Try another possible path
            default_config = "d:/HyperVesion/UEFI/U-E-F-I/PUBG-2/.shared/pubg_config.hpp";
        }
        
        if (!fs::exists(default_module)) {
            default_module = "d:/HyperVesion/UEFI/U-E-F-I/offset_scanner/Process-Dumper/bin/dump_TslGame.exe";
        }

        if (!fs::exists(default_config) || !fs::exists(default_module)) {
            std::cerr << "[-] Auto-scan failed: Files not found at default paths.\n";
            std::cerr << "    Config: " << default_config << "\n";
            std::cerr << "    Module: " << default_module << "\n\n";
            PrintUsage();
            return 2;
        }

        std::cout << "[+] Found Config: " << default_config << "\n";
        std::cout << "[+] Found Module: " << default_module << "\n\n";

        CliArgs auto_args;
        auto_args.positional.push_back("scan-config");
        auto_args.options["config"].push_back(default_config);
        auto_args.options["module"].push_back(default_module);
        
        // Manual dispatch to scan-config wrapper logic
        int rc = CommandBulkGenerate(auto_args);
        if (rc != 0) return rc;
        
        std::cout << "\n[*] Validating Generated Signatures...\n";
        CliArgs scan_args;
        scan_args.positional.push_back("scan");
        scan_args.options["rules"].push_back("rules_auto");
        scan_args.options["module"].push_back(default_module);
        return CommandScan(scan_args);
    }

    if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
        PrintUsage();
        return 0;
    }

    CliArgs args = ParseArgs(argc, argv);
    if (args.positional.empty()) {
        std::cout << "[INFO] No command provided. Showing help.\n\n";
        PrintUsage();
        return 0;
    }

    const std::string command = args.positional.front();
    if (command == "extract") {
        return CommandExtract(args);
    }
    if (command == "mask") {
        return CommandMask(args);
    }
    if (command == "validate") {
        return CommandValidate(args);
    }
    if (command == "scan") {
        return CommandScan(args);
    }
    if (command == "find-refs") {
        return CommandFindRefs(args);
    }
    if (command == "bulk-generate") {
        return CommandBulkGenerate(args);
    }
    if (command == "scan-config") {
        // Just a convenience wrapper that runs bulk-generate then scan
        int rc = CommandBulkGenerate(args);
        if (rc != 0) return rc;
        
        std::string rules_dir = GetOption(args, "rules-out").value_or("rules");
        CliArgs scan_args;
        scan_args.positional.push_back("scan");
        scan_args.options["rules"].push_back(rules_dir);
        scan_args.options["module"].push_back(*GetOption(args, "module"));
        return CommandScan(scan_args);
    }
    if (command == "help") {
        PrintUsage();
        return 0;
    }
    if (command == "find-imm") {
        return CommandFindImm(args);
    }
    if (command == "extract-offset") {
        return CommandExtractOffset(args);
    }
    if (command == "make-sig") {
        return CommandMakeSig(args);
    }

    std::cerr << "[-] Unknown command: " << command << "\n";
    PrintUsage();
    return 2;
}
