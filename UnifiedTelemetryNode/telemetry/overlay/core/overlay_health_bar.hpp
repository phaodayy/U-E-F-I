#pragma once

#include "../../imgui/imgui.h"
#include <algorithm>
#include <cmath>

namespace OverlayHealthBar {

inline int AlphaByte(float alpha) {
    alpha = std::clamp(alpha, 0.0f, 1.0f);
    return static_cast<int>(alpha * 255.0f);
}

inline ImU32 WithAlpha(ImU32 color, float alphaMult) {
    const int alpha = static_cast<int>(((color >> 24) & 0xFF) * std::clamp(alphaMult, 0.0f, 1.0f));
    return (color & 0x00FFFFFF) | (alpha << 24);
}

inline ImU32 Tint(ImU32 color, float amount) {
    amount = std::clamp(amount, 0.0f, 1.0f);
    const int r = static_cast<int>((color & 0xFF) + (255 - (color & 0xFF)) * amount);
    const int g = static_cast<int>(((color >> 8) & 0xFF) + (255 - ((color >> 8) & 0xFF)) * amount);
    const int b = static_cast<int>(((color >> 16) & 0xFF) + (255 - ((color >> 16) & 0xFF)) * amount);
    const int a = (color >> 24) & 0xFF;
    return IM_COL32(r, g, b, a);
}

inline ImU32 Shade(ImU32 color, float amount) {
    amount = std::clamp(amount, 0.0f, 1.0f);
    const int r = static_cast<int>((color & 0xFF) * (1.0f - amount));
    const int g = static_cast<int>(((color >> 8) & 0xFF) * (1.0f - amount));
    const int b = static_cast<int>(((color >> 16) & 0xFF) * (1.0f - amount));
    const int a = (color >> 24) & 0xFF;
    return IM_COL32(r, g, b, a);
}

inline void GetFillRect(const ImVec2& min, const ImVec2& max, bool vertical, float pct, ImVec2& fillMin, ImVec2& fillMax) {
    pct = std::clamp(pct, 0.0f, 1.0f);
    if (vertical) {
        const float h = max.y - min.y;
        fillMin = ImVec2(min.x, max.y - h * pct);
        fillMax = max;
    } else {
        const float w = max.x - min.x;
        fillMin = min;
        fillMax = ImVec2(min.x + w * pct, max.y);
    }
}

inline void DrawSegmented(ImDrawList* draw, const ImVec2& pMin, const ImVec2& pMax, float pct,
                          ImU32 fillColor, bool vertical, float alphaMult) {
    const int segments = 6;
    const float gap = 1.4f;
    const float radius = 2.0f;
    draw->AddRectFilled(pMin, pMax, IM_COL32(5, 7, 10, AlphaByte(0.58f * alphaMult)), 4.0f);
    draw->AddRect(pMin, pMax, IM_COL32(255, 255, 255, AlphaByte(0.12f * alphaMult)), 4.0f, 0, 1.0f);

    const float filled = pct * segments;
    for (int i = 0; i < segments; ++i) {
        ImVec2 a;
        ImVec2 b;
        if (vertical) {
            const float h = (pMax.y - pMin.y - gap * (segments + 1)) / segments;
            a = ImVec2(pMin.x + 2.0f, pMax.y - gap - (i + 1) * h - i * gap);
            b = ImVec2(pMax.x - 2.0f, a.y + h);
        } else {
            const float w = (pMax.x - pMin.x - gap * (segments + 1)) / segments;
            a = ImVec2(pMin.x + gap + i * (w + gap), pMin.y + 2.0f);
            b = ImVec2(a.x + w, pMax.y - 2.0f);
        }

        const float segFill = std::clamp(filled - static_cast<float>(i), 0.0f, 1.0f);
        draw->AddRectFilled(a, b, IM_COL32(18, 22, 28, AlphaByte(0.82f * alphaMult)), radius);
        if (segFill > 0.0f) {
            ImVec2 fa;
            ImVec2 fb;
            GetFillRect(a, b, vertical, segFill, fa, fb);
            draw->AddRectFilled(fa, fb, WithAlpha(fillColor, alphaMult), radius);
        }
    }
}

inline void Draw(ImDrawList* draw, const ImVec2& pMin, const ImVec2& pMax, float pct,
                 ImU32 fillColor, bool vertical, int style, float alphaMult, bool warning) {
    if (!draw) return;
    pct = std::clamp(pct, 0.0f, 1.0f);
    style = std::clamp(style, 0, 4);

    const float width = pMax.x - pMin.x;
    const float height = pMax.y - pMin.y;
    if (width <= 1.0f || height <= 1.0f) return;

    const float thickness = vertical ? width : height;
    const float radius = std::clamp(thickness * 0.50f, 2.0f, 6.0f);
    const ImVec2 innerMin(pMin.x + 1.0f, pMin.y + 1.0f);
    const ImVec2 innerMax(pMax.x - 1.0f, pMax.y - 1.0f);
    ImVec2 fillMin;
    ImVec2 fillMax;
    GetFillRect(innerMin, innerMax, vertical, pct, fillMin, fillMax);

    const ImU32 fill = WithAlpha(fillColor, alphaMult);
    const ImU32 fillLight = WithAlpha(Tint(fillColor, 0.30f), alphaMult);
    const ImU32 fillDark = WithAlpha(Shade(fillColor, 0.32f), alphaMult);
    const ImU32 bg = IM_COL32(6, 9, 13, AlphaByte(0.62f * alphaMult));
    const ImU32 edge = IM_COL32(255, 255, 255, AlphaByte(0.12f * alphaMult));

    if (style == 1) {
        DrawSegmented(draw, pMin, pMax, pct, fillColor, vertical, alphaMult);
        return;
    }

    if (style == 2) {
        draw->AddRectFilled(ImVec2(pMin.x + 1.0f, pMin.y + 1.0f), ImVec2(pMax.x + 1.0f, pMax.y + 1.0f),
            IM_COL32(0, 0, 0, AlphaByte(0.20f * alphaMult)), radius);
        draw->AddRectFilled(pMin, pMax, IM_COL32(8, 12, 17, AlphaByte(0.46f * alphaMult)), radius);
        draw->AddRect(pMin, pMax, IM_COL32(195, 220, 255, AlphaByte(0.16f * alphaMult)), radius, 0, 1.0f);
        if (vertical) {
            draw->AddRectFilledMultiColor(fillMin, fillMax, fillLight, fillLight, fillDark, fillDark);
            draw->AddLine(ImVec2(pMin.x + width * 0.36f, pMin.y + 2.0f), ImVec2(pMin.x + width * 0.36f, pMax.y - 2.0f),
                IM_COL32(255, 255, 255, AlphaByte(0.18f * alphaMult)), 1.0f);
        } else {
            draw->AddRectFilledMultiColor(fillMin, fillMax, fillDark, fillLight, fillLight, fillDark);
            draw->AddLine(ImVec2(pMin.x + 2.0f, pMin.y + height * 0.38f), ImVec2(pMax.x - 2.0f, pMin.y + height * 0.38f),
                IM_COL32(255, 255, 255, AlphaByte(0.18f * alphaMult)), 1.0f);
        }
        return;
    }

    if (style == 3) {
        draw->AddRectFilled(pMin, pMax, IM_COL32(4, 6, 9, AlphaByte(0.76f * alphaMult)), 2.0f);
        draw->AddRectFilled(fillMin, fillMax, fill, 1.5f);
        const int ticks = 4;
        for (int i = 1; i < ticks; ++i) {
            const float t = static_cast<float>(i) / ticks;
            if (vertical) {
                const float y = pMax.y - (pMax.y - pMin.y) * t;
                draw->AddLine(ImVec2(pMin.x, y), ImVec2(pMax.x, y), IM_COL32(0, 0, 0, AlphaByte(0.52f * alphaMult)), 1.0f);
            } else {
                const float x = pMin.x + (pMax.x - pMin.x) * t;
                draw->AddLine(ImVec2(x, pMin.y), ImVec2(x, pMax.y), IM_COL32(0, 0, 0, AlphaByte(0.52f * alphaMult)), 1.0f);
            }
        }
        draw->AddRect(pMin, pMax, IM_COL32(255, 255, 255, AlphaByte(0.18f * alphaMult)), 2.0f, 0, 1.0f);
        return;
    }

    if (style == 4) {
        const float pulse = warning ? (0.5f + 0.5f * std::sin(static_cast<float>(ImGui::GetTime()) * 8.0f)) : 0.0f;
        draw->AddRectFilled(ImVec2(pMin.x - 1.0f, pMin.y - 1.0f), ImVec2(pMax.x + 1.0f, pMax.y + 1.0f),
            WithAlpha(fillColor, (0.12f + pulse * 0.10f) * alphaMult), radius + 1.0f);
        draw->AddRectFilled(pMin, pMax, IM_COL32(5, 8, 12, AlphaByte(0.68f * alphaMult)), radius);
        draw->AddRectFilled(fillMin, fillMax, fill, radius);
        draw->AddRect(fillMin, fillMax, fillLight, radius, 0, 1.0f);
        draw->AddRect(pMin, pMax, warning ? IM_COL32(255, 80, 58, AlphaByte((0.26f + pulse * 0.30f) * alphaMult)) : edge,
            radius, 0, 1.0f);
        return;
    }

    draw->AddRectFilled(ImVec2(pMin.x + 1.0f, pMin.y + 1.0f), ImVec2(pMax.x + 1.0f, pMax.y + 1.0f),
        IM_COL32(0, 0, 0, AlphaByte(0.22f * alphaMult)), radius);
    draw->AddRectFilled(pMin, pMax, bg, radius);
    draw->AddRect(pMin, pMax, edge, radius, 0, 1.0f);
    draw->AddRectFilled(fillMin, fillMax, fill, radius);
    if (vertical) {
        draw->AddRectFilled(fillMin, ImVec2(fillMin.x + (fillMax.x - fillMin.x) * 0.45f, fillMax.y),
            IM_COL32(255, 255, 255, AlphaByte(0.13f * alphaMult)), radius);
    } else {
        draw->AddRectFilled(fillMin, ImVec2(fillMax.x, fillMin.y + (fillMax.y - fillMin.y) * 0.45f),
            IM_COL32(255, 255, 255, AlphaByte(0.13f * alphaMult)), radius);
    }
}

} // namespace OverlayHealthBar
