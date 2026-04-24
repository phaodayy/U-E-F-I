#include "menu.h"
#include "translation.hpp"
#include "../imgui/imgui_internal.h"
#include "../utils/config.h"
#include "../overlay/overlay_menu.hpp"
#include "../sdk/driver.hpp"
#include <string>
#include <vector>

namespace CustomWidgets {

static void StyledHeader(const char* label) {
    ImGui::TextColored(GUI::main_color, label);
    ImGui::Separator();
    ImGui::Spacing();
}

static bool StyledCheckbox(const char* label, bool* v) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.12f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.18f, 0.18f, 0.18f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.24f, 0.24f, 0.24f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, GUI::main_color);
    
    bool result = ImGui::Checkbox(label, v);
    
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(1);
    return result;
}

static bool StyledSliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.1f") {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 5.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.12f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, GUI::main_color);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(GUI::main_color.x * 0.8f, GUI::main_color.y * 0.8f, GUI::main_color.z * 0.8f, 1.0f));
    
    ImGui::Text(label);
    // ImGui::SameLine(180); // Removed to fix column overlapping
    ImGui::PushItemWidth(-1);
    bool result = ImGui::SliderFloat((std::string("##") + label).c_str(), v, v_min, v_max, format);
    ImGui::PopItemWidth();
    
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    return result;
}

static bool StyledSliderInt(const char* label, int* v, int v_min, int v_max) {
    float f = (float)*v;
    if (StyledSliderFloat(label, &f, (float)v_min, (float)v_max, "%.0f")) {
        *v = (int)f;
        return true;
    }
    return false;
}

static bool StyledCombo(const char* label, int* current_item, const char* const items[], int items_count) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.12f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Header, GUI::main_color);
    
    ImGui::Text(label);
    // ImGui::SameLine(180); // Removed to fix column overlapping
    ImGui::PushItemWidth(-1);
    bool result = ImGui::Combo((std::string("##") + label).c_str(), current_item, items, items_count);
    ImGui::PopItemWidth();
    
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(1);
    return result;
}

static bool StyledButton(const char* label, const ImVec2& size = ImVec2(-1, 0)) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(GUI::main_color.x, GUI::main_color.y, GUI::main_color.z, 0.4f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(GUI::main_color.x, GUI::main_color.y, GUI::main_color.z, 0.6f));
    
    bool result = ImGui::Button(label, size);
    
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(1);
    return result;
}

} // namespace CustomWidgets

namespace GUI {

ImVec4 main_color = ImVec4(0.0f, 1.0f, 0.8f, 1.0f); // Cyan accent
int current_language = 0;
int current_tab = 0;
int selected_color_item = 0;

// Features
bool esp_toggle = true;
bool esp_box = true;
int esp_box_type = 0;
bool esp_fillbox = false;
bool esp_head_circle = false;
bool esp_skeleton = true;
bool esp_health = true;
int esp_health_type = 2; // Left
bool esp_distance = true;
bool esp_name = true;
bool esp_snapline = false;
bool esp_weapon = false;
bool esp_team_check = true;
int snapline_type = 0;

bool flick_toggle = false;
int flick_key = VK_CONTROL;
float flick_fov = 10.0f;
float flick_smooth = 1.0f;
int flick_bone = 0;
bool flick_auto_fire = false;
float flick_speed = 3.0f;
float flick_deadzone = 1.0f;

bool macro_toggle = false;
float macro_intensity_x = 0.0f;
float macro_intensity_y = 2.0f;
int macro_delay = 10;

bool esp_radar = false;
float radar_pos_x = 100.0f;
float radar_pos_y = 100.0f;
float radar_size = 150.0f;
float radar_zoom = 100.0f;

// Colors
float box_visible_color[4] = {0.0f, 1.0f, 0.0f, 1.0f};
float invisible_color[4] = {1.0f, 0.5f, 0.0f, 1.0f};
float box_invisible_color[4] = {1.0f, 0.5f, 0.0f, 1.0f};
float skeleton_visible_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
float fill_visible_color[4] = {0.0f, 0.0f, 0.0f, 0.25f};
float snapline_visible_color[4] = {1.0f, 1.0f, 0.0f, 1.0f};
float head_circle_visible_color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
bool use_global_invisible_color = true;
float skeleton_invisible_color[4] = {1.0f, 0.5f, 0.0f, 1.0f};
float fill_invisible_color[4] = {1.0f, 0.5f, 0.0f, 0.25f};
float snapline_invisible_color[4] = {1.0f, 0.5f, 0.0f, 1.0f};
float head_circle_invisible_color[4] = {1.0f, 0.5f, 0.0f, 1.0f};
float radar_dot_color[4] = {1.0f, 0.0f, 0.0f, 1.0f};

// Compatibility
float box_fill_color[4] = {0.0f, 0.0f, 0.0f, 0.25f};
float snapline_color[4] = {1.0f, 1.0f, 0.0f, 1.0f};
float head_circle_color[4] = {1.0f, 0.0f, 0.0f, 1.0f};

float skel_thickness = 1.5f;
float skel_alpha = 100.0f;
float box_alpha = 100.0f;
float fill_alpha = 25.0f;
int render_distance = 300;
int render_sleep = 5;
bool anti_screenshot = true;

static void SidebarTab(const char* label, int id) {
    bool active = (current_tab == id);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.2f, 0.5f));
    
    if (active) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(main_color.x, main_color.y, main_color.z, 0.15f));
        ImGui::PushStyleColor(ImGuiCol_Text, main_color);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    }

    if (ImGui::Button(label, ImVec2(170, 48))) current_tab = id;
    
    if (active) {
        ImVec2 pMin = ImGui::GetItemRectMin();
        ImVec2 pMax = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRectFilled(pMin, ImVec2(pMin.x + 4, pMax.y), ImGui::GetColorU32(main_color));
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

void RenderAimbotContent() {
    auto Lang = Translation::Get();
    CustomWidgets::StyledHeader(Lang.AimbotHeader);
    
    CustomWidgets::StyledCheckbox(Lang.EnableFlickbot, &flick_toggle);
    ImGui::Spacing();
    
    if (flick_toggle) {
        CustomWidgets::StyledSliderFloat(Lang.TargetFOV, &flick_fov, 1.0f, 50.0f);
        CustomWidgets::StyledSliderFloat(Lang.SmoothFactor, &flick_smooth, 1.0f, 20.0f);
        CustomWidgets::StyledSliderFloat(Lang.AimStrength, &flick_speed, 0.1f, 20.0f);
        CustomWidgets::StyledSliderFloat(Lang.Deadzone, &flick_deadzone, 0.0f, 5.0f);
        
        const char* bones_en[] = {"Head", "Neck", "Chest", "Pelvis"};
        const char* bones_vn[] = {"Dau", "Co", "Nguc", "Hong"};
        const char* const* bones = (current_language == 1) ? bones_vn : bones_en;
        CustomWidgets::StyledCombo(Lang.TargetBone, &flick_bone, bones, 4);
        
        const char* keys_en[] = {"Left Click", "Right Click", "Mouse 4", "Mouse 5", "CTRL", "ALT", "SHIFT"};
        const char* keys_vn[] = {"Chuot trai", "Chuot phai", "Chuot 4", "Chuot 5", "CTRL", "ALT", "SHIFT"};
        const char* const* keys = (current_language == 1) ? keys_vn : keys_en;

        int key_vals[] = {VK_LBUTTON, VK_RBUTTON, VK_XBUTTON1, VK_XBUTTON2, VK_CONTROL, VK_MENU, VK_SHIFT};
        int selected_key = 0;
        for(int i=0; i<7; i++) if(flick_key == key_vals[i]) selected_key = i;
        if(CustomWidgets::StyledCombo(Lang.TriggerKey, &selected_key, keys, 7)) flick_key = key_vals[selected_key];
        
        CustomWidgets::StyledCheckbox(Lang.AutoFire, &flick_auto_fire);
    }

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
    CustomWidgets::StyledHeader(Lang.MacroHeader);
    CustomWidgets::StyledCheckbox(Lang.EnableMacro, &macro_toggle);
    if (macro_toggle) {
        CustomWidgets::StyledSliderFloat(Lang.VerticalIntensity, &macro_intensity_y, 0.0f, 10.0f);
        CustomWidgets::StyledSliderFloat(Lang.HorizontalIntensity, &macro_intensity_x, -5.0f, 5.0f);
        CustomWidgets::StyledSliderInt(Lang.MacroDelay, &macro_delay, 1, 50);
        ImGui::TextDisabled(Lang.MacroNote);
    }
}

void RenderVisualsContent() {
    auto Lang = Translation::Get();
    CustomWidgets::StyledHeader(Lang.VisualsHeader);

    ImGui::Columns(2, "EspColumns", false);
    CustomWidgets::StyledCheckbox(Lang.MasterESP, &esp_toggle);
    CustomWidgets::StyledCheckbox(Lang.PlayerBoxes, &esp_box);
    CustomWidgets::StyledCheckbox(Lang.Skeleton, &esp_skeleton);
    CustomWidgets::StyledCheckbox(Lang.Health, &esp_health);
    CustomWidgets::StyledCheckbox(Lang.HeadMarker, &esp_head_circle);
    CustomWidgets::StyledCheckbox(Lang.Names, &esp_name);
    CustomWidgets::StyledCheckbox(Lang.Distance, &esp_distance);
    
    ImGui::NextColumn();
    CustomWidgets::StyledCheckbox(Lang.TeamCheck, &esp_team_check);
    if (esp_box) {
        const char* types_en[] = {"Cornered", "2D Full", "3D Cube"};
        const char* types_vn[] = {"Kieu goc", "Khung 2D", "Khung 3D"};
        const char* const* types = (current_language == 1) ? types_vn : types_en;
        CustomWidgets::StyledCombo(Lang.BoxStyle, &esp_box_type, types, 3);
    }
    CustomWidgets::StyledSliderInt(Lang.MaxDistance, &render_distance, 50, 600);
    ImGui::Columns(1);

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    if (ImGui::TreeNode(Lang.ColorTree)) {
        const char* items_en[] = {"Box Lines", "Skeleton", "Box Fill", "Snaplines", "Head Dots"};
        const char* items_vn[] = {"Vien khung", "Xuong", "Nen khung", "Snaplines", "Dau dau"};
        const char* const* color_items = (current_language == 1) ? items_vn : items_en;
        CustomWidgets::StyledCombo(Lang.TargetItem, &selected_color_item, color_items, 5);
        
        float* vis = nullptr;
        float* inv = (use_global_invisible_color) ? invisible_color : nullptr;

        switch(selected_color_item) {
            case 0: vis = box_visible_color; if(!inv) inv = box_invisible_color; break;
            case 1: vis = skeleton_visible_color; if(!inv) inv = skeleton_invisible_color; break;
            case 2: vis = fill_visible_color; if(!inv) inv = fill_invisible_color; break;
            case 3: vis = snapline_visible_color; if(!inv) inv = snapline_invisible_color; break;
            case 4: vis = head_circle_visible_color; if(!inv) inv = head_circle_invisible_color; break;
        }

        ImGui::ColorEdit4(Lang.ColorVisible, vis, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        ImGui::ColorEdit4(Lang.ColorInvisible, inv, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        CustomWidgets::StyledCheckbox(Lang.SyncGlobal, &use_global_invisible_color);
        ImGui::TreePop();
    }
}

void RenderMiscContent() {
    auto Lang = Translation::Get();
    CustomWidgets::StyledHeader(Lang.SystemHeader);

    const char* lang_items[] = {"English", "Vietnamese (Khong dau)"};
    if (CustomWidgets::StyledCombo("Language / Ngon ngu", &current_language, lang_items, 2)) {
        Translation::CurrentLanguage = current_language;
    }
    ImGui::Spacing();

    ImGui::TextDisabled(Lang.KernelTest);
    if (CustomWidgets::StyledButton(Lang.RunMouse, ImVec2(180, 35))) {
        for (int i = 0; i < 20; i++) { Driver::MoveMouse(4, 4); Sleep(5); }
        for (int i = 0; i < 20; i++) { Driver::MoveMouse(-4, -4); Sleep(5); }
    }
    ImGui::SameLine();
    if (CustomWidgets::StyledButton(Lang.RunClick, ImVec2(180, 35))) {
        Driver::Click();
    }

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    if (CustomWidgets::StyledCheckbox(Lang.StreamProof, &anti_screenshot)) {
        g_Menu.UpdateAntiScreenshot();
    }
    CustomWidgets::StyledCheckbox(Lang.MiniRadar, &esp_radar);
    
    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    if (CustomWidgets::StyledButton(Lang.SaveConfig, ImVec2(180, 40))) SaveConfig();
    ImGui::SameLine();
    if (CustomWidgets::StyledButton(Lang.LoadConfig, ImVec2(180, 40))) LoadConfig();
    
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), Lang.CoreInfo);
}

void RenderMainMenu(bool *menu_open, float window_alpha) {
    if (!*menu_open) return;

    ImGui::SetNextWindowSize(ImVec2(660, 480), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    
    ImGui::Begin("##GZ_MAIN", menu_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    
    ImVec2 p = ImGui::GetWindowPos();
    ImVec2 s = ImGui::GetWindowSize();
    
    // Background Glow / Border
    ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + s.x, p.y + s.y), ImGui::GetColorU32(main_color), 12.0f, 0, 1.5f);

    auto Lang = Translation::Get();
    Translation::CurrentLanguage = current_language;

    // sidebar container
    ImGui::BeginChild("##Sidebar", ImVec2(170, 0), true, ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::Spacing(); ImGui::Spacing();
        ImGui::SetCursorPosX(25);
        ImGui::TextColored(main_color, Lang.MainTitle);
        ImGui::SetCursorPosX(25);
        ImGui::TextDisabled(Lang.Subtitle);
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
        
        SidebarTab(Lang.TabAimbot, 0);
        SidebarTab(Lang.TabVisuals, 1);
        SidebarTab(Lang.TabSettings, 2);

        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 50);
        ImGui::SetCursorPosX(20);
        if (CustomWidgets::StyledButton(Lang.ExitCheat, ImVec2(130, 32))) exit(0);
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // main container
    ImGui::BeginChild("##ContentFrame", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
    {
        ImGui::SetCursorPos(ImVec2(25, 25));
        
        ImGui::BeginGroup();
        if (current_tab == 0) RenderAimbotContent();
        else if (current_tab == 1) RenderVisualsContent();
        else if (current_tab == 2) RenderMiscContent();
        ImGui::EndGroup();
    }
    ImGui::EndChild();

    ImGui::End();
    ImGui::PopStyleVar(4);
}

void SaveConfig() {
  auto &config = Config::ConfigManager::Get();
  config.SetBool("esp_toggle", esp_toggle);
  config.SetBool("esp_box", esp_box);
  config.SetInt("esp_box_type", esp_box_type);
  config.SetBool("esp_fillbox", esp_fillbox);
  config.SetBool("esp_head_circle", esp_head_circle);
  config.SetBool("esp_skeleton", esp_skeleton);
  config.SetBool("esp_health", esp_health);
  config.SetInt("esp_health_type", esp_health_type);
  config.SetBool("esp_distance", esp_distance);
  config.SetBool("esp_name", esp_name);
  config.SetBool("esp_snapline", esp_snapline);
  config.SetBool("esp_weapon", esp_weapon);
  config.SetBool("esp_team_check", esp_team_check);
  config.SetBool("esp_radar", esp_radar);
  config.SetInt("snapline_type", snapline_type);
  config.SetFloat("skel_thickness", skel_thickness);
  config.SetFloat("skel_alpha", skel_alpha);
  config.SetFloat("box_alpha", box_alpha);
  config.SetFloat("fill_alpha", fill_alpha);
  config.SetInt("render_distance", render_distance);
  config.SetInt("render_sleep", render_sleep);
  config.SetBool("anti_screenshot", anti_screenshot);
  config.SetBool("flick_toggle", flick_toggle);
  config.SetInt("flick_key", flick_key);
  config.SetFloat("flick_fov", flick_fov);
  config.SetFloat("flick_smooth", flick_smooth);
  config.SetInt("flick_bone", flick_bone);
  config.SetBool("flick_auto_fire", flick_auto_fire);
  config.SetFloat("flick_deadzone", flick_deadzone);
  config.SetBool("macro_toggle", macro_toggle);
  config.SetFloat("macro_intensity_x", macro_intensity_x);
  config.SetFloat("macro_intensity_y", macro_intensity_y);
  config.SetInt("macro_delay", macro_delay);
  config.SetBool("use_global_invisible_color", use_global_invisible_color);
  
  config.SetInt("current_language", current_language);
  config.Save("config.ini");
}

void LoadConfig() {
  auto &config = Config::ConfigManager::Get();
  config.Load("config.ini");
  esp_toggle = config.GetBool("esp_toggle", esp_toggle);
  esp_box = config.GetBool("esp_box", esp_box);
  esp_box_type = config.GetInt("esp_box_type", esp_box_type);
  esp_fillbox = config.GetBool("esp_fillbox", esp_fillbox);
  esp_head_circle = config.GetBool("esp_head_circle", esp_head_circle);
  esp_skeleton = config.GetBool("esp_skeleton", esp_skeleton);
  esp_health = config.GetBool("esp_health", esp_health);
  esp_health_type = config.GetInt("esp_health_type", esp_health_type);
  esp_distance = config.GetBool("esp_distance", esp_distance);
  esp_name = config.GetBool("esp_name", esp_name);
  esp_snapline = config.GetBool("esp_snapline", esp_snapline);
  esp_weapon = config.GetBool("esp_weapon", esp_weapon);
  esp_team_check = config.GetBool("esp_team_check", esp_team_check);
  esp_radar = config.GetBool("esp_radar", esp_radar);
  snapline_type = config.GetInt("snapline_type", snapline_type);
  skel_thickness = config.GetFloat("skel_thickness", skel_thickness);
  skel_alpha = config.GetFloat("skel_alpha", skel_alpha);
  box_alpha = config.GetFloat("box_alpha", box_alpha);
  fill_alpha = config.GetFloat("fill_alpha", fill_alpha);
  render_distance = (int)config.GetFloat("render_distance", (float)render_distance);
  render_sleep = config.GetInt("render_sleep", render_sleep);
  anti_screenshot = config.GetBool("anti_screenshot", anti_screenshot);
  flick_toggle = config.GetBool("flick_toggle", flick_toggle);
  flick_key = config.GetInt("flick_key", flick_key);
  flick_fov = config.GetFloat("flick_fov", flick_fov);
  flick_smooth = config.GetFloat("flick_smooth", flick_smooth);
  flick_bone = config.GetInt("flick_bone", flick_bone);
  flick_auto_fire = config.GetBool("flick_auto_fire", flick_auto_fire);
  flick_deadzone = config.GetFloat("flick_deadzone", flick_deadzone);
  macro_toggle = config.GetBool("macro_toggle", macro_toggle);
  macro_intensity_x = config.GetFloat("macro_intensity_x", macro_intensity_x);
  macro_intensity_y = config.GetFloat("macro_intensity_y", macro_intensity_y);
  macro_delay = config.GetInt("macro_delay", macro_delay);
  use_global_invisible_color = config.GetBool("use_global_invisible_color", use_global_invisible_color);
  current_language = config.GetInt("current_language", current_language);
  Translation::CurrentLanguage = current_language;
}

} // namespace GUI
