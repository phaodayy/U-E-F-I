#include "overlay_menu.hpp"
void OverlayMenu::SetupStyle() {
    auto& style = ImGui::GetStyle();

    // --- Advanced Layout & Rounding ---
    style.TabRounding       = 8.0f;
    style.ScrollbarSize     = 6.0f;
    style.GrabMinSize       = 25.0f;

    style.WindowBorderSize  = 0.0f;
    style.ChildBorderSize   = 0.0f;
    style.FrameBorderSize   = 1.0f;

    style.WindowPadding     = ImVec2(20, 20);
    style.FramePadding      = ImVec2(12, 6);
    style.ItemSpacing       = ImVec2(12, 12);
    style.ItemInnerSpacing  = ImVec2(8, 8);

    style.WindowTitleAlign  = ImVec2(0.5f, 0.5f);

    // --- Palette: Deep Sea & Electric Cyan ---
    ImVec4* colors = style.Colors;

    // Core surfaces
    colors[ImGuiCol_WindowBg]             = ImVec4(0.02f, 0.01f, 0.05f, 0.90f); // Deep near-black blue
    colors[ImGuiCol_ChildBg]              = ImVec4(0.05f, 0.10f, 0.20f, 0.35f); // Transparent blue card
    colors[ImGuiCol_PopupBg]              = ImVec4(0.02f, 0.04f, 0.08f, 1.00f);
    colors[ImGuiCol_Border]               = ImVec4(0.00f, 0.80f, 1.00f, 0.25f); // Electric Cyan border

    // Logic controls
    colors[ImGuiCol_FrameBg]              = ImVec4(0.05f, 0.15f, 0.25f, 0.50f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.00f, 0.80f, 1.00f, 0.15f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.00f, 0.80f, 1.00f, 0.30f);

    // Title & Headers
    colors[ImGuiCol_TitleBg]              = ImVec4(0.02f, 0.01f, 0.05f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.04f, 0.08f, 0.15f, 1.00f);

    // Selection highlight
    colors[ImGuiCol_Header]               = ImVec4(0.00f, 0.80f, 1.00f, 0.25f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.00f, 0.80f, 1.00f, 0.40f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(0.00f, 0.80f, 1.00f, 0.60f);

    // Buttons (Cyber Cyan)
    colors[ImGuiCol_Button]               = ImVec4(0.00f, 0.80f, 1.00f, 0.10f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.00f, 0.80f, 1.00f, 0.35f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.00f, 0.80f, 1.00f, 0.55f);

    colors[ImGuiCol_SliderGrab]           = ImVec4(0.00f, 0.80f, 1.00f, 0.70f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.20f, 0.90f, 1.00f, 1.00f);

    colors[ImGuiCol_CheckMark]            = ImVec4(0.00f, 0.80f, 1.00f, 1.00f);

    // Scrollbar styling - matched to Glass theme
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.00f, 0.80f, 1.00f, 0.40f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.00f, 0.80f, 1.00f, 0.70f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.00f, 0.80f, 1.00f, 0.90f);

    colors[ImGuiCol_Text]                 = ImVec4(0.90f, 0.95f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]         = ImVec4(0.40f, 0.50f, 0.60f, 1.00f);

    colors[ImGuiCol_Separator]            = ImVec4(0.00f, 0.80f, 1.00f, 0.20f);
}
