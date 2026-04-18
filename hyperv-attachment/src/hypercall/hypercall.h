#pragma once
#include <structures/trap_frame.h>
#include <cstdint>

union hypercall_info_t;

namespace hypercall
{
	void process(hypercall_info_t hypercall_info, trap_frame_t* trap_frame);
}

// === EPT Mouse Hook Shared Globals (Ring -1) ===
extern volatile std::int32_t g_pending_mouse_x;
extern volatile std::int32_t g_pending_mouse_y;
extern std::uint64_t         g_mouse_callback_va;
extern std::uint64_t         g_mouse_shadow_page_va;
extern std::uint64_t         g_mouse_func_offset;


