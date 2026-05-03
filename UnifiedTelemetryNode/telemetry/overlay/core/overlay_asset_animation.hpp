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

    const float frameWidth = static_cast<float>(icon->Width);
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
    (void)options;
    if (!draw || !icon || !icon->SRV || icon->Width <= 0 || icon->Height <= 0) return;

    const float width = rectMax.x - rectMin.x;
    const float height = rectMax.y - rectMin.y;
    if (width <= 0.0f || height <= 0.0f) return;

    const int frameCount = (std::max)(1, icon->Frames);
    const float uvStep = 1.0f / static_cast<float>(frameCount);
    draw->AddImage((ImTextureID)icon->SRV, rectMin, rectMax,
        ImVec2(0.0f, 0.0f), ImVec2(uvStep, 1.0f), tint);
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
