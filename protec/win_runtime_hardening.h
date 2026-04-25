#pragma once

#include <Windows.h>

namespace protec
{
    struct hardening_options_t
    {
        bool enable_heap_termination = true;
        bool harden_dll_search_order = true;
        bool disable_extension_points = true;
        bool block_remote_images = true;
        bool block_low_integrity_images = true;
        bool prefer_system32_images = true;
        bool disable_dynamic_code = false;
    };

    struct hardening_result_t
    {
        bool heap_termination_enabled = false;
        bool dll_search_order_hardened = false;
        bool extension_points_disabled = false;
        bool image_load_policy_applied = false;
        bool dynamic_code_policy_applied = false;
        DWORD last_error = ERROR_SUCCESS;
    };

    hardening_result_t apply_baseline_hardening(const hardening_options_t& options = {});
    bool is_debugger_likely_present();
    void secure_wipe(void* buffer, SIZE_T size);
}
