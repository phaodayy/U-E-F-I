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
    std::cout << "  extract  --module <path> --anchor-rva <hex> --out <extract.json> [--radius <n>]\n";
    std::cout << "  mask     --in <extract.json> --out <rule.json> [--name <rule_name>] [--module-name <name>]\n";
    std::cout << "  validate --rule <rule.json> --module <file> [--module <file> ...]\n";
    std::cout << "  scan     --rules <dir> --module <file>\n";
}
} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "[INFO] No command provided. Showing help.\n\n";
        PrintUsage();
        return 0;
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
    if (command == "help") {
        PrintUsage();
        return 0;
    }

    std::cerr << "[-] Unknown command: " << command << "\n";
    PrintUsage();
    return 2;
}
