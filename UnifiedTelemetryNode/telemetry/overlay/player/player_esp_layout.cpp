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

ImVec2 Stack::TakeTop(const ImVec2& size, float gap, int rowIndex) {
    if (topRow_.index != rowIndex) {
        if (topRow_.index != -1) topOffset_ += topRow_.maxHeight + gap;
        topRow_.index = rowIndex;
        topRow_.offset = 0.0f;
        topRow_.maxHeight = 0.0f;
    }
    
    // For simplicity, we center the entire group later? No, let's just pack them horizontally for now.
    // To center properly, we'd need to know the total width of the row.
    // For now, we just offset from the center.
    const float x = (left_ + right_ - size.x) * 0.5f + topRow_.offset;
    const float y = top_ - topOffset_ - size.y;
    
    topRow_.offset += size.x + gap;
    if (size.y > topRow_.maxHeight) topRow_.maxHeight = size.y;
    
    return ImVec2(x, y);
}

ImVec2 Stack::TakeBottom(const ImVec2& size, float gap, int rowIndex) {
    if (bottomRow_.index != rowIndex) {
        if (bottomRow_.index != -1) bottomOffset_ += bottomRow_.maxHeight + gap;
        bottomRow_.index = rowIndex;
        bottomRow_.offset = 0.0f;
        bottomRow_.maxHeight = 0.0f;
    }

    const float x = (left_ + right_ - size.x) * 0.5f + bottomRow_.offset;
    const float y = bottom_ + bottomOffset_;
    
    bottomRow_.offset += size.x + gap;
    if (size.y > bottomRow_.maxHeight) bottomRow_.maxHeight = size.y;
    
    return ImVec2(x, y);
}

ImVec2 Stack::Take(Side side, const ImVec2& size, float gap, int rowIndex) {
    switch (side) {
    case Side::Left: {
        if (leftRow_.index != rowIndex) {
            if (leftRow_.index != -1) leftOffset_ += leftRow_.maxWidth + gap;
            leftRow_.index = rowIndex;
            leftRow_.offset = 0.0f;
            leftRow_.maxWidth = 0.0f;
        }
        const float x = left_ - leftOffset_ - size.x;
        const float y = top_ + leftRow_.offset;
        leftRow_.offset += size.y + gap;
        if (size.x > leftRow_.maxWidth) leftRow_.maxWidth = size.x;
        return ImVec2(x, y);
    }
    case Side::Right: {
        if (rightRow_.index != rowIndex) {
            if (rightRow_.index != -1) rightOffset_ += rightRow_.maxWidth + gap;
            rightRow_.index = rowIndex;
            rightRow_.offset = 0.0f;
            rightRow_.maxWidth = 0.0f;
        }
        const float x = right_ + rightOffset_;
        const float y = top_ + rightRow_.offset;
        rightRow_.offset += size.y + gap;
        if (size.x > rightRow_.maxWidth) rightRow_.maxWidth = size.x;
        return ImVec2(x, y);
    }
    case Side::Top:
        return TakeTop(size, gap, rowIndex);
    case Side::Bottom:
        return TakeBottom(size, gap, rowIndex);
    }

    return TakeTop(size, gap, rowIndex);
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
