#pragma once

#include "overlay_menu.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace OverlayAssetAnimation {

struct DrawOptions {
    bool hovered = false;
    bool selected = false;
    bool important = false;
    bool glow = true;
    bool shine = true;
    float alpha = 1.0f;
    float strength = 1.0f;
};

inline int AlphaByte(float alpha) {
    alpha = std::clamp(alpha, 0.0f, 1.0f);
    return static_cast<int>(alpha * 255.0f);
}

inline ImU32 WithAlpha(ImU32 color, float alphaMult) {
    const int alpha = static_cast<int>(((color >> 24) & 0xFF) * std::clamp(alphaMult, 0.0f, 1.0f));
    return (color & 0x00FFFFFF) | (alpha << 24);
}

inline float TexturePhase(const TextureInfo* icon, const ImVec2& center) {
    std::uintptr_t seed = icon && icon->SRV ? reinterpret_cast<std::uintptr_t>(icon->SRV) : 0u;
    seed ^= static_cast<std::uintptr_t>(center.x * 17.0f);
    seed ^= static_cast<std::uintptr_t>(center.y * 31.0f);
    return static_cast<float>(seed % 997u) * 0.006301f;
}

inline ImVec2 FitSize(const TextureInfo* icon, float targetSize) {
    if (!icon || icon->Width <= 0 || icon->Height <= 0 || targetSize <= 0.0f) {
        return ImVec2(0.0f, 0.0f);
    }

    const int frameCount = (std::max)(1, icon->Frames);
    const float frameWidth = static_cast<float>(icon->Width) / static_cast<float>(frameCount);
    float width = targetSize;
    float height = targetSize;
    const float aspect = frameWidth / static_cast<float>(icon->Height);
    if (aspect > 1.0f) {
        height = targetSize / aspect;
    } else {
        width = targetSize * aspect;
    }
    return ImVec2(width, height);
}

inline void DrawAnimatedImageRect(ImDrawList* draw, TextureInfo* icon,
                                  const ImVec2& rectMin, const ImVec2& rectMax,
                                  ImU32 tint = IM_COL32(255, 255, 255, 255),
                                  DrawOptions options = {}) {
    if (!draw || !icon || !icon->SRV || icon->Width <= 0 || icon->Height <= 0) return;

    const float width = rectMax.x - rectMin.x;
    const float height = rectMax.y - rectMin.y;
    if (width <= 0.0f || height <= 0.0f) return;

    const ImVec2 center((rectMin.x + rectMax.x) * 0.5f, (rectMin.y + rectMax.y) * 0.5f);
    const bool enabled = g_Menu.asset_animation_enabled;
    const float baseStrength = enabled ?
        std::clamp(g_Menu.asset_animation_strength * options.strength, 0.0f, 2.0f) : 0.0f;
    const float speed = std::clamp(g_Menu.asset_animation_speed, 0.10f, 3.00f);
    const float phase = TexturePhase(icon, center);
    const float time = static_cast<float>(ImGui::GetTime());
    const int frameCount = (std::max)(1, icon->Frames);
    int frameIndex = 0;
    if (enabled && frameCount > 1) {
        frameIndex = static_cast<int>(time * 12.0f * speed + phase * 7.0f) % frameCount;
    }
    const float pulse = std::sin(time * 2.70f * speed + phase);
    const float drift = std::sin(time * 1.45f * speed + phase * 1.7f);
    const float emphasis = (options.hovered ? 0.65f : 0.0f) +
        (options.selected ? 0.55f : 0.0f) +
        (options.important ? 0.45f : 0.0f);
    const float scale = 1.0f + (0.018f + emphasis * 0.012f) * baseStrength * pulse;
    const float bobY = baseStrength * drift * 0.85f;
    const float alpha = std::clamp(options.alpha * (1.0f + 0.075f * baseStrength * pulse), 0.0f, 1.0f);

    const ImVec2 imageMin(
        center.x - width * scale * 0.5f,
        center.y - height * scale * 0.5f + bobY);
    const ImVec2 imageMax(
        center.x + width * scale * 0.5f,
        center.y + height * scale * 0.5f + bobY);

    if (enabled && baseStrength > 0.01f && options.glow && g_Menu.asset_animation_glow) {
        const float glowAlpha = (0.055f + 0.045f * (pulse * 0.5f + 0.5f)) *
            (1.0f + emphasis) * alpha;
        draw->AddRectFilled(
            ImVec2(imageMin.x - 2.0f, imageMin.y - 2.0f),
            ImVec2(imageMax.x + 2.0f, imageMax.y + 2.0f),
            IM_COL32(0, 0, 0, AlphaByte(glowAlpha)),
            6.0f);
    }

    const float uvStep = 1.0f / static_cast<float>(frameCount);
    const ImVec2 uvMin(uvStep * static_cast<float>(frameIndex), 0.0f);
    const ImVec2 uvMax(uvStep * static_cast<float>(frameIndex + 1), 1.0f);

    draw->AddImage((ImTextureID)icon->SRV, imageMin, imageMax, uvMin, uvMax, WithAlpha(tint, alpha));

    if (enabled && baseStrength > 0.01f && options.shine && g_Menu.asset_animation_shine) {
        const float travel = std::fmod(time * 0.38f * speed + phase * 0.19f, 1.0f);
        const float shineX = imageMin.x - width * 0.55f + travel * width * 2.10f;
        draw->PushClipRect(imageMin, imageMax, true);
        draw->AddLine(
            ImVec2(shineX, imageMax.y),
            ImVec2(shineX + height * 0.75f, imageMin.y),
            IM_COL32(255, 255, 255, AlphaByte((0.12f + emphasis * 0.03f) * alpha * baseStrength)),
            (std::max)(1.0f, width * 0.055f));
        draw->PopClipRect();
    }
}

inline void DrawStaticImageRect(ImDrawList* draw, TextureInfo* icon,
                                const ImVec2& rectMin, const ImVec2& rectMax,
                                ImU32 tint = IM_COL32(255, 255, 255, 255),
                                float alpha = 1.0f) {
    if (!draw || !icon || !icon->SRV || icon->Width <= 0 || icon->Height <= 0) return;

    const int frameCount = (std::max)(1, icon->Frames);
    const float uvStep = 1.0f / static_cast<float>(frameCount);
    draw->AddImage((ImTextureID)icon->SRV, rectMin, rectMax,
        ImVec2(0.0f, 0.0f), ImVec2(uvStep, 1.0f), WithAlpha(tint, alpha));
}

inline void DrawStaticImage(ImDrawList* draw, TextureInfo* icon, const ImVec2& center,
                            float targetSize,
                            ImU32 tint = IM_COL32(255, 255, 255, 255),
                            float alpha = 1.0f) {
    const ImVec2 size = FitSize(icon, targetSize);
    if (size.x <= 0.0f || size.y <= 0.0f) return;

    DrawStaticImageRect(draw, icon,
        ImVec2(center.x - size.x * 0.5f, center.y - size.y * 0.5f),
        ImVec2(center.x + size.x * 0.5f, center.y + size.y * 0.5f),
        tint,
        alpha);
}

inline void DrawAnimatedImage(ImDrawList* draw, TextureInfo* icon, const ImVec2& center,
                              float targetSize,
                              ImU32 tint = IM_COL32(255, 255, 255, 255),
                              DrawOptions options = {}) {
    const ImVec2 size = FitSize(icon, targetSize);
    if (size.x <= 0.0f || size.y <= 0.0f) return;

    DrawAnimatedImageRect(draw, icon,
        ImVec2(center.x - size.x * 0.5f, center.y - size.y * 0.5f),
        ImVec2(center.x + size.x * 0.5f, center.y + size.y * 0.5f),
        tint,
        options);
}

} // namespace OverlayAssetAnimation
