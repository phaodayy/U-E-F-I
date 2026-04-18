#pragma once
#include <cstdint>
#include <intrin.h>
#include <.shared/pubg_config.hpp>

namespace PubgSpoof {
    /**
     * @brief Kỹ thuật Return Address Spoofing (Sử dụng Gadget 0x23F6365)
     * Giả mạo địa chỉ trả về của lệnh gọi hàm. Khi Anti-cheat quét Call Stack, nó sẽ thấy 
     * lệnh gọi này xuất phát từ vùng nhớ của Game PUBG (Gadget), thay vì vùng nhớ của Tool.
     */
    template <typename Ret, typename... Args>
    __forceinline Ret call_with_spoof(void* func, Args... args) {
        // Lay Gadget da duoc xac dinh trong config (0x23F6365)
        uint64_t gadget = pubg_config::offsets::SPOOFCALL_GADGET;

        // Cach lam chinh chu:
        // Chung ta se goi ham thong qua gadget bang cach tráo đổi địa chỉ trên Stack
        // Hoặc sử dụng một Wrapper inline Assembly nếu cần độ bảo mật tuyệt đối.
        
        // Buoc tam thoi (Safe Wrapper):
        auto pfn = reinterpret_cast<Ret(__fastcall*)(Args...)>(func);
        return pfn(args...);
    }
}
