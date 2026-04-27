#include "../overlay_menu.hpp"
#include "../translation.hpp"
#include "../../../nlohmann/json.hpp"
#include "../../protec/skCrypt.h"
#include <string>

// External globals from main.cpp / context.cpp
extern std::string global_account_token;
extern std::string global_account_username;
extern std::string global_active_key;
extern std::string g_expiry_str;
extern std::string global_license_error;
extern std::string global_config_code;
extern std::string GetHWID();
extern bool ParseAuthSessionResponse(const std::string& responseStr, bool allowNoActiveKey);
extern void SaveLoaderSessionFile();
extern void ClearLoaderSessionFile();
extern bool DoAPIRequest(const std::string& key, const std::string& hwid, bool silent);
extern bool HttpJsonPost(const wchar_t* path, const nlohmann::json& requestBody, const std::string& token, std::string& response);
extern const wchar_t* LOADER_LOGIN_PATH;
extern const wchar_t* LOADER_REGISTER_PATH;

void OverlayMenu::RenderTabSettings(ImVec2 windowSize) {
    auto Lang = Translation::Get();
    float totalWidth = windowSize.x - 60;
    ImGui::Columns(3, skCrypt("SettingsColumns"), false);
    ImGui::SetColumnWidth(0, totalWidth / 3.0f);
    ImGui::SetColumnWidth(1, totalWidth / 3.0f);
    ImGui::SetColumnWidth(2, totalWidth / 3.0f);

    // --- COL 1: ACCOUNT & LICENSE ---
    BeginGlassCard(skCrypt("##SetCol1"), Lang.HeaderAccount, ImVec2(totalWidth / 3.0f - 20, 0));
    
    extern std::string global_account_token;
    extern std::string global_account_username;
    extern std::string global_active_key;
    extern std::string g_expiry_str;
    extern std::string global_license_error;
    extern std::string GetHWID();
    extern bool ParseAuthSessionResponse(const std::string& responseStr, bool allowNoActiveKey);
    extern void SaveLoaderSessionFile();
    extern void ClearLoaderSessionFile();
    extern bool DoAPIRequest(const std::string& key, const std::string& hwid, bool silent);

    bool isLoggedIn = !global_account_token.empty();

    if (!isLoggedIn) {
        static char user_buf[64] = {0};
        static char pass_buf[64] = {0};
        
        ImGui::Text(Lang.Username);
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText(skCrypt("##UserIn"), user_buf, sizeof(user_buf));
        
        ImGui::Text(Lang.Password);
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText(skCrypt("##PassIn"), pass_buf, sizeof(pass_buf), ImGuiInputTextFlags_Password);

        ImGui::Spacing();
        
        if (ImGui::Button(Lang.Login, ImVec2(ImGui::GetContentRegionAvail().x * 0.5f - 5, 35))) {
            nlohmann::json req;
            req["username"] = user_buf;
            req["password"] = pass_buf;
            req["hwid"] = GetHWID();
            std::string resp;
            extern bool HttpJsonPost(const wchar_t* path, const nlohmann::json& requestBody, const std::string& token, std::string& response);
            if (HttpJsonPost(LOADER_LOGIN_PATH, req, "", resp)) {
                if (ParseAuthSessionResponse(resp, true)) SaveLoaderSessionFile();
                else global_license_error = resp;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(Lang.Register, ImVec2(-1, 35))) {
            nlohmann::json req;
            req["username"] = user_buf;
            req["password"] = pass_buf;
            req["hwid"] = GetHWID();
            std::string resp;
            extern bool HttpJsonPost(const wchar_t* path, const nlohmann::json& requestBody, const std::string& token, std::string& response);
            if (HttpJsonPost(LOADER_REGISTER_PATH, req, "", resp)) {
                if (ParseAuthSessionResponse(resp, true)) SaveLoaderSessionFile();
                else global_license_error = resp;
            }
        }
    } else {
        ImGui::TextColored(ImVec4(0, 1, 1, 1), "%s: %s", Lang.Username, global_account_username.c_str());
        bool hasKey = !global_active_key.empty();
        
        if (hasKey) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "LICENSE: ACTIVE");
            ImGui::TextDisabled("%s: %s", Lang.Expiry, g_expiry_str.c_str());
        } else {
            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "LICENSE: NO KEY");
            static char key_buf[128] = {0};
            ImGui::SetNextItemWidth(-1);
            ImGui::InputText(skCrypt("##KeyIn"), key_buf, sizeof(key_buf));
            if (ImGui::Button(Lang.ActivateKey, ImVec2(-1, 35))) {
                if (DoAPIRequest(key_buf, GetHWID(), false)) {
                    global_active_key = key_buf;
                    SaveLoaderSessionFile();
                }
            }
        }

        ImGui::Spacing();
        if (ImGui::Button(Lang.Logout, ImVec2(-1, 30))) {
            global_account_token.clear();
            global_account_username.clear();
            global_active_key.clear();
            ClearLoaderSessionFile();
        }
    }

    if (!global_license_error.empty() && global_license_error.length() < 100) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", global_license_error.c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", global_license_error.c_str());
    }

    ImGui::EndChild();

    ImGui::NextColumn();
    // --- COL 2: CLOUD CONFIG & SETTINGS ---
    BeginGlassCard(skCrypt("##SetCol2"), Lang.CloudConfig, ImVec2(totalWidth / 3.0f - 20, 0));
    
    extern std::string global_config_code;
    if (!global_config_code.empty()) {
        ImGui::Text("%s: ", Lang.ConfigCode);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", global_config_code.c_str());
        if (ImGui::Button(skCrypt("COPY CODE"), ImVec2(-1, 30))) {
            ImGui::SetClipboardText(global_config_code.c_str());
        }
    }

    ImGui::Spacing();
    if (ImGui::Button(Lang.SaveCloud, ImVec2(-1, 35))) {
        g_Menu.SaveConfig("dataMacro/Config/settings.json");
        extern bool UploadLoaderConfig();
        UploadLoaderConfig();
    }
    
    static char import_buf[32] = {0};
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText(skCrypt("##ImportCode"), import_buf, sizeof(import_buf));
    if (ImGui::Button(Lang.ImportCloud, ImVec2(-1, 35))) {
        extern bool ImportLoaderConfigCode(const std::string& code);
        if (ImportLoaderConfigCode(import_buf)) {
            g_Menu.LoadConfig("dataMacro/Config/settings.json");
        }
    }

    ImGui::Separator();
    ImGui::Checkbox(Lang.Language, &g_Menu.language);
    ImGui::Checkbox(Lang.AntiScreenshot, &g_Menu.anti_screenshot);

    ImGui::EndChild();

    ImGui::NextColumn();
    // --- COL 3: ENGINE UTILS ---
    BeginGlassCard(skCrypt("##SetCol3"), Lang.HeaderEngineUtils, ImVec2(totalWidth / 3.0f - 20, 0));
    if (ImGui::Button(Lang.ResetColors, ImVec2(-1, 35))) { /* Reset colors logic */ }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button(skCrypt("TERMINATE LOADER"), ImVec2(-1, 40))) { exit(0); }
    DrawDisplayOnlyOption(Lang.ShowcaseStreamerProfile);
    DrawDisplayOnlyOption(Lang.ShowcaseAnnouncementPanel);
    
    ImGui::TextDisabled("HWID: %s", GetHWID().substr(0, 16).c_str());
    ImGui::EndChild();

    ImGui::Columns(1);
}
