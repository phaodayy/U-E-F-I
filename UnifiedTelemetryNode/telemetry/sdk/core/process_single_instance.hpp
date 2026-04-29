#pragma once

#include <windows.h>

namespace ProcessSingleInstance {

class Guard {
public:
    Guard() = default;
    Guard(const Guard&) = delete;
    Guard& operator=(const Guard&) = delete;
    ~Guard();

    bool Acquire();

private:
    HANDLE mutex_ = nullptr;
    HANDLE lock_file_ = INVALID_HANDLE_VALUE;
};

} // namespace ProcessSingleInstance
