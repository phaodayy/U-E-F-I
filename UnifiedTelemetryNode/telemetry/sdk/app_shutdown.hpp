#pragma once

#include <atomic>

namespace AppShutdown {

inline std::atomic_bool& RequestedFlag() {
    static std::atomic_bool requested{ false };
    return requested;
}

inline void Request() {
    RequestedFlag().store(true, std::memory_order_release);
}

inline bool IsRequested() {
    return RequestedFlag().load(std::memory_order_acquire);
}

} // namespace AppShutdown
