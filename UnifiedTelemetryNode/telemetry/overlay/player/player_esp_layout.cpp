#include "player_esp_layout.hpp"

namespace PlayerEspLayout {

Stack::Stack(float left, float top, float right, float bottom)
    : left_(left), top_(top), right_(right), bottom_(bottom) {}

BarSlot Stack::TakeBar(Side side, float thickness, float gap) {
    BarSlot slot{};

    switch (side) {
    case Side::Left:
        slot.Min = ImVec2(left_ - leftOffset_ - thickness, top_);
        slot.Max = ImVec2(left_ - leftOffset_, bottom_);
        leftOffset_ += thickness + gap;
        break;
    case Side::Right:
        slot.Min = ImVec2(right_ + rightOffset_, top_);
        slot.Max = ImVec2(right_ + rightOffset_ + thickness, bottom_);
        rightOffset_ += thickness + gap;
        break;
    case Side::Top:
        slot.Horizontal = true;
        slot.Min = ImVec2(left_, top_ - topOffset_ - thickness);
        slot.Max = ImVec2(right_, top_ - topOffset_);
        topOffset_ += thickness + gap;
        break;
    case Side::Bottom:
        slot.Horizontal = true;
        slot.Min = ImVec2(left_, bottom_ + bottomOffset_);
        slot.Max = ImVec2(right_, bottom_ + bottomOffset_ + thickness);
        bottomOffset_ += thickness + gap;
        break;
    }

    return slot;
}

ImVec2 Stack::TakeTop(const ImVec2& size, float gap) {
    topOffset_ += size.y;
    const float x = (left_ + right_ - size.x) * 0.5f;
    const float y = top_ - topOffset_;
    topOffset_ += gap;
    return ImVec2(x, y);
}

ImVec2 Stack::TakeBottom(const ImVec2& size, float gap) {
    const float x = (left_ + right_ - size.x) * 0.5f;
    const float y = bottom_ + bottomOffset_;
    bottomOffset_ += size.y + gap;
    return ImVec2(x, y);
}

ImVec2 Stack::Take(Side side, const ImVec2& size, float gap) {
    switch (side) {
    case Side::Left: {
        const float x = left_ - leftOffset_ - size.x;
        const float y = top_ + leftTextOffset_;
        leftTextOffset_ += size.y + gap;
        return ImVec2(x, y);
    }
    case Side::Right: {
        const float x = right_ + rightOffset_;
        const float y = top_ + rightTextOffset_;
        rightTextOffset_ += size.y + gap;
        return ImVec2(x, y);
    }
    case Side::Top:
        return TakeTop(size, gap);
    case Side::Bottom:
        return TakeBottom(size, gap);
    }

    return TakeTop(size, gap);
}

Side SideFromMenu(int value) {
    switch (value) {
    case 0: return Side::Left;
    case 1: return Side::Right;
    case 2: return Side::Top;
    case 3: return Side::Bottom;
    default: return Side::Left;
    }
}

} // namespace PlayerEspLayout
