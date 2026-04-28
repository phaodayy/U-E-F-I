#pragma once

#include "../../imgui/imgui.h"

namespace PlayerEspLayout {

enum class Side {
    Left = 0,
    Right = 1,
    Top = 2,
    Bottom = 3
};

struct BarSlot {
    ImVec2 Min;
    ImVec2 Max;
    bool Horizontal = false;
};

class Stack {
public:
    Stack(float left, float top, float right, float bottom);

    BarSlot TakeBar(Side side, float thickness, float gap);
    ImVec2 Take(Side side, const ImVec2& size, float gap);
    ImVec2 TakeTop(const ImVec2& size, float gap);
    ImVec2 TakeBottom(const ImVec2& size, float gap);

private:
    float left_ = 0.0f;
    float top_ = 0.0f;
    float right_ = 0.0f;
    float bottom_ = 0.0f;
    float topOffset_ = 2.0f;
    float bottomOffset_ = 2.0f;
    float leftOffset_ = 2.0f;
    float rightOffset_ = 2.0f;
    float leftTextOffset_ = 0.0f;
    float rightTextOffset_ = 0.0f;
};

Side SideFromMenu(int value);

} // namespace PlayerEspLayout
