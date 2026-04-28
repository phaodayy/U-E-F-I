#pragma once

#include "translation_strings.hpp"

namespace Translation {

inline const char* GetAttachmentName(int type, int id) {

        if (id == 0) return (CurrentLanguage == 1) ? skCrypt("Khong") : skCrypt("None");



        if (CurrentLanguage == 1) { // Vietnamese

            if (type == 0) { // Scope

                switch(id) {

                    case 1: return skCrypt("Cham do");

                    case 2: return skCrypt("Holo");

                    case 3: return skCrypt("2X");

                    case 4: return skCrypt("3X");

                    case 5: return skCrypt("4X");

                    case 6: return skCrypt("6X");

                    case 7: return skCrypt("8X");

                    case 8: return skCrypt("15X");

                }

            } else if (type == 1) { // Muzzle

                switch(id) {

                    case 1: return skCrypt("Lua");

                    case 2: return skCrypt("Thanh");

                    case 4: return skCrypt("Giat");

                }

            } else if (type == 2) { // Grip

                switch(id) {

                    case 1: return skCrypt("Doc");

                    case 2: return skCrypt("Ngang");

                    case 3: return skCrypt("Nua");

                    case 4: return skCrypt("Nhe");

                }

            }

        } else { // English

            if (type == 0) { // Scope

                switch(id) {

                    case 1: return skCrypt("RedDot");

                    case 2: return skCrypt("Holo");

                    case 3: return skCrypt("2X");

                    case 4: return skCrypt("3X");

                    case 5: return skCrypt("4X");

                    case 6: return skCrypt("6X");

                    case 7: return skCrypt("8X");

                    case 8: return skCrypt("15X");

                }

            } else if (type == 1) { // Muzzle

                switch(id) {

                    case 1: return skCrypt("Flash");

                    case 2: return skCrypt("Supp");

                    case 4: return skCrypt("Comp");

                }

            } else if (type == 2) { // Grip

                switch(id) {

                    case 1: return skCrypt("Vertical");

                    case 2: return skCrypt("Angled");

                    case 3: return skCrypt("Half");

                    case 4: return skCrypt("Light");

                }

            }

        }

        return (CurrentLanguage == 1) ? skCrypt("Khong") : skCrypt("None");

    }

} // namespace Translation