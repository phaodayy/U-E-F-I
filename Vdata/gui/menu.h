#pragma once
#include "../imgui/imgui.h"

// Custom ImGui extensions (static to avoid linker conflicts)
namespace ImGui {
static bool tab(const char *label, bool selected);
static bool subtab(const char *label, bool selected);
static bool MenuChild(const char *str_id, const ImVec2 &size_arg, bool sub_tab,
                      ImGuiWindowFlags extra_flags);
} // namespace ImGui

namespace GUI {
extern int current_language; // 0: EN, 1: VN
// Menu colors
extern ImVec4 main_color;

// Menu state
extern int current_tab;
extern int current_subtab;
extern int selected_color_item;

// Visuals features
extern bool v_enable;
extern bool v_frame;
extern int v_frame_type;
extern bool v_frame_fill;
extern bool v_point;
extern bool v_bone;
extern bool v_stat;
extern int v_stat_type; // 0=Top, 1=Bottom, 2=Left, 3=Right
extern bool v_dist;
extern bool v_alias;
extern bool v_line;
extern bool v_tool;
extern bool v_friend;
extern bool v_map;
extern float radar_pos_x;
extern float radar_pos_y;
extern float radar_size;
extern float radar_zoom;
extern int v_line_type;

// Assist features
extern bool a_enable;
extern int a_key;
extern float a_fov;
extern float a_smooth;
extern int a_target; // 0=Head, 1=Neck, 2=Chest
extern bool a_trigger;
extern float a_speed;
extern float a_deadzone;

// Macro features
extern bool macro_toggle;
extern float macro_intensity_x;
extern float macro_intensity_y;
extern int macro_delay;

// ESP colors - each feature has its own visible color, but shares one invisible
// color
extern float box_visible_color[4];
extern float box_invisible_color[4];
extern float invisible_color[4]; // Global invisible color for all features
extern float skeleton_visible_color[4];
extern float skeleton_invisible_color[4];
extern float fill_visible_color[4];
extern float fill_invisible_color[4];
extern float snapline_visible_color[4];
extern float snapline_invisible_color[4];
extern float head_circle_visible_color[4];
extern float head_circle_invisible_color[4];
extern bool use_global_invisible_color;
extern float box_fill_color[4];
extern float snapline_color[4];
extern float head_circle_color[4];
extern float radar_dot_color[4];

// Visuals settings
extern float b_thickness;
extern float b_alpha;
extern float f_alpha;
extern float ff_alpha;
extern int render_distance;
extern int render_sleep;
extern bool anti_screenshot;

// Menu functions
void RenderMainMenu(bool *menu_open, float window_alpha);
void DrawBackground(ImVec2 pos, ImVec2 size);
void DrawHeader(ImVec2 pos, ImVec2 size);
void RenderTabs();
void RenderVisualsContent();
void RenderColorsContent();
void RenderMiscContent();
void SaveConfig();
void LoadConfig();
} // namespace GUI
