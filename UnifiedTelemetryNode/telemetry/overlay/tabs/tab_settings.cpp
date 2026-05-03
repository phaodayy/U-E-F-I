#include "../core/overlay_menu.hpp"
#include "../entities/entity_aliases.hpp"
#include "../core/overlay_presets.hpp"
#include "../translation/translation.hpp"
#include "../../../nlohmann/json.hpp"
#include <protec/skCrypt.h>
#include <string>

// External globals from main.cpp / context.cpp
extern std::string global_account_token;
extern std::string global_account_username;
extern std::string global_account_role;
extern std::string global_active_key;
extern std::string g_expiry_str;
extern std::string global_license_error;
extern std::string global_config_code;
extern std::string GetHWID();
extern void ClearActiveEntitlementState();
extern bool HasActiveLoaderEntitlement();
extern bool ParseAuthSessionResponse(const std::string& responseStr, bool allowNoActiveKey);
extern void SaveLoaderSessionFile();
extern void ClearLoaderSessionFile();
extern bool DoAPIRequest(const std::string& key, const std::string& hwid, bool silent);
extern bool DownloadLoaderConfig();
extern bool HttpJsonPost(const wchar_t* path, const nlohmann::json& requestBody, const std::string& token, std::string& response);
extern const wchar_t* LOADER_LOGIN_PATH;

void OverlayMenu::RenderTabSettings(ImVec2 windowSize) {
    auto Lang = Translation::Get();
    float totalWidth = windowSize.x - 60;
    bool isLoggedIn = !global_account_token.empty();
    if (isLoggedIn) {
        ImGui::Columns(3, skCrypt("SettingsColumns"), false);
        ImGui::SetColumnWidth(0, totalWidth / 3.0f);
        ImGui::SetColumnWidth(1, totalWidth / 3.0f);
        ImGui::SetColumnWidth(2, totalWidth / 3.0f);
    }

    // --- COL 1: ACCOUNT & LICENSE ---
    const float accountCardWidth = isLoggedIn ? totalWidth / 3.0f - 20.0f : totalWidth - 20.0f;
    BeginGlassCard(skCrypt("##SetCol1"), Lang.HeaderAccount, ImVec2(accountCardWidth, 0));
    
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
        
        if (ImGui::Button(Lang.Login, ImVec2(-1, 35))) {
            nlohmann::json req;
            req["username"] = user_buf;
            req["password"] = pass_buf;
            req["hwid"] = GetHWID();
            std::string resp;
            if (HttpJsonPost(LOADER_LOGIN_PATH, req, "", resp)) {
                if (ParseAuthSessionResponse(resp, true)) {
                    if (HasActiveLoaderEntitlement()) {
                        DownloadLoaderConfig();
                    }
                    SaveLoaderSessionFile();
                } else {
                    global_license_error = resp;
                }
            }
        }
    } else {
        ImGui::TextColored(ImVec4(0, 1, 1, 1), "%s: %s", Lang.Username, global_account_username.c_str());
        bool hasKey = HasActiveLoaderEntitlement();
        static bool show_redeem_key = false;
        static char key_buf[128] = {0};
        
        if (hasKey) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "LICENSE: ACTIVE");
            ImGui::TextDisabled("%s: %s", Lang.Expiry, g_expiry_str.c_str());
            if (ImGui::Button(show_redeem_key ? "Huy nap key" : "Nap them key", ImVec2(-1, 35))) {
                show_redeem_key = !show_redeem_key;
            }
        } else {
            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "LICENSE: EXPIRED / NO ACTIVE TIME");
            show_redeem_key = true;
        }

        if (show_redeem_key) {
            ImGui::SetNextItemWidth(-1);
            ImGui::InputText(skCrypt("##KeyIn"), key_buf, sizeof(key_buf));
            if (ImGui::Button(Lang.ActivateKey, ImVec2(-1, 35))) {
                if (DoAPIRequest(key_buf, GetHWID(), false)) {
                    DownloadLoaderConfig();
                    SaveLoaderSessionFile();
                    key_buf[0] = '\0';
                    show_redeem_key = false;
                }
            }
        }

        ImGui::Spacing();
        if (ImGui::Button(Lang.Logout, ImVec2(-1, 30))) {
            global_account_token.clear();
            global_account_username.clear();
            global_account_role.clear();
            global_config_code.clear();
            ClearActiveEntitlementState();
            ClearLoaderSessionFile();
        }
    }

    if (!global_license_error.empty() && global_license_error.length() < 100) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", global_license_error.c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", global_license_error.c_str());
    }

    ImGui::EndChild();
    if (!isLoggedIn) {
        return;
    }

    ImGui::NextColumn();
    // --- COL 2: CLOUD CONFIG & SETTINGS ---
    BeginGlassCard(skCrypt("##SetCol2"), Lang.CloudConfig, ImVec2(totalWidth / 3.0f - 20, 0));
    
    if (!global_config_code.empty()) {
        ImGui::Text("%s: ", Lang.ConfigCode);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", global_config_code.c_str());
        if (ImGui::Button(skCrypt("COPY CODE"), ImVec2(-1, 30))) {
            ImGui::SetClipboardText(global_config_code.c_str());
        }
    }

    ImGui::Spacing();
    
    double currentTime = ImGui::GetTime();
    double elapsed = currentTime - cloud_save_last_time;
    double timeLeft = 60.0 - elapsed;
    bool canSave = (timeLeft <= 0.0);

    if (!canSave) ImGui::BeginDisabled();

    char saveLabel[128];
    if (canSave) {
        strcpy_s(saveLabel, Lang.SaveCloud);
    } else {
        sprintf_s(saveLabel, skCrypt("%s (%ds)"), Lang.SaveCloud, (int)ceil(timeLeft));
    }

    if (ImGui::Button(saveLabel, ImVec2(-1, 35))) {
        cloud_save_last_time = currentTime;
        extern bool UploadLoaderConfig();
        UploadLoaderConfig();
    }

    if (!canSave) ImGui::EndDisabled();
    
    static char import_buf[32] = {0};
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText(skCrypt("##ImportCode"), import_buf, sizeof(import_buf));
    if (ImGui::Button(Lang.ImportCloud, ImVec2(-1, 35))) {
        extern bool ImportLoaderConfigCode(const std::string& code);
        ImportLoaderConfigCode(import_buf);
    }

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 0.4f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 0.6f));
    if (ImGui::Button(skCrypt("RESET ALL SETTINGS"), ImVec2(-1, 35))) {
        extern bool ImportLoaderConfigCode(const std::string& code);
        ImportLoaderConfigCode(skCrypt("CFG-3Y4K4U2K6R216M4L"));
    }
    ImGui::PopStyleColor(2);

    ImGui::Separator();
    
    const char* languages[] = { "English", "Vietnamese" };
    ImGui::SetNextItemWidth(-1);
    if (ImGui::Combo(Lang.Language, &g_Menu.language, languages, IM_ARRAYSIZE(languages))) {
        Translation::CurrentLanguage = g_Menu.language;
    }
    
    ImGui::Checkbox(Lang.AntiScreenshot, &g_Menu.anti_screenshot);

    ImGui::EndChild();

    ImGui::NextColumn();
    // --- COL 3: ENGINE UTILS ---
    BeginGlassCard(skCrypt("##SetCol3"), Lang.HeaderEngineUtils, ImVec2(totalWidth / 3.0f - 20, 0));
    if (ImGui::Button(Lang.ResetColors, ImVec2(-1, 35))) { /* Reset colors logic */ }

    ImGui::TextDisabled(skCrypt("DISPLAY PRESETS"));
    if (ImGui::Button(skCrypt("Clean"), ImVec2(-1, 28))) {
        OverlayPresets::Apply(g_Menu, OverlayPresets::Preset::Clean);
    }
    if (ImGui::Button(skCrypt("Loot"), ImVec2(-1, 28))) {
        OverlayPresets::Apply(g_Menu, OverlayPresets::Preset::Loot);
    }
    if (ImGui::Button(skCrypt("Combat"), ImVec2(-1, 28))) {
        OverlayPresets::Apply(g_Menu, OverlayPresets::Preset::Combat);
    }

    ImGui::Spacing();
    if (ImGui::Button(skCrypt("RELOAD ENTITY ALIASES"), ImVec2(-1, 30))) {
        EntityAliases::Reload();
    }
    const std::string& aliasPath = EntityAliases::LoadedPath();
    ImGui::TextDisabled(skCrypt("Aliases: %s"), aliasPath.empty() ? "not loaded" : aliasPath.c_str());
    
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
