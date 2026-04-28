#pragma once

#include "translation_strings.hpp"
#include "translation_get_vn.hpp"
#include "translation_get_en.hpp"
#include "translation_attachments.hpp"

namespace Translation {

inline Strings Get() {
    Strings s{};
    if (CurrentLanguage == 1) {
        FillVietnamese(s);
    } else {
        FillEnglish(s);
    }
    return s;
}

} // namespace Translation