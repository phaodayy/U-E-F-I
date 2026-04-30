#include "hypercall_bridge.hpp"

#include <shared/hypercall/hypercall_def.h>

extern "C" std::uint64_t launch_raw_hypercall(hypercall_info_t rcx,
                                              std::uint64_t rdx,
                                              std::uint64_t r8,
                                              std::uint64_t r9);

namespace
{
    static std::uint64_t current_primary_key = hypercall_default_primary_key;
    static std::uint64_t current_secondary_key = hypercall_default_secondary_key;
    static bool is_initialized = false;
}

std::uint64_t telemetryHyperCall::MakeHypercall(const hypercall_type_t call_type,
                                                const std::uint64_t call_reserved_data,
                                                const std::uint64_t rdx,
                                                const std::uint64_t r8,
                                                const std::uint64_t r9)
{
    hypercall_info_t hypercall_info = {};
    hypercall_info.primary_key = current_primary_key;
    hypercall_info.secondary_key = current_secondary_key;
    hypercall_info.call_type = call_type;
    hypercall_info.call_reserved_data = call_reserved_data;

    return launch_raw_hypercall(hypercall_info, rdx, r8, r9);
}

bool telemetryHyperCall::Init()
{
    if (is_initialized) return true;

    hypercall_info_t hypercall_info = {};
    hypercall_info.primary_key = current_primary_key;
    hypercall_info.secondary_key = current_secondary_key;
    hypercall_info.call_type = hypercall_type_t::_hc_0x210;
    hypercall_info.call_reserved_data = 0;

    // Obfuscated magic: use centralized seeds from hypercall_def.h
    std::uint64_t result = launch_raw_hypercall(hypercall_info, hypercall_magic_seed_a ^ hypercall_magic_seed_b, 0, 0);
    if (result != 0)
    {
        current_primary_key = (result >> 16) & 0xFFFF;
        current_secondary_key = result & 0x7F;
        is_initialized = true;
        return true;
    }
    return false;
}

std::uint64_t telemetryHyperCall::ReadGuestVirtualMemory(void* const guest_destination_buffer,
                                                    const std::uint64_t guest_source_virtual_address,
                                                    const std::uint64_t source_cr3,
                                                    const std::uint64_t size)
{
    virt_memory_op_hypercall_info_t memory_op_call = {};
    memory_op_call.call_type = hypercall_type_t::_hc_0x120;
    memory_op_call.memory_operation = memory_operation_t::read_operation;
    memory_op_call.address_of_page_directory = source_cr3 >> 12;

    hypercall_info_t hypercall_info = {};
    hypercall_info.value = memory_op_call.value;
    const auto guest_destination_virtual_address = reinterpret_cast<std::uint64_t>(guest_destination_buffer);

    return MakeHypercall(hypercall_info.call_type,
                         hypercall_info.call_reserved_data,
                         guest_destination_virtual_address,
                         guest_source_virtual_address,
                         size);
}

std::uint64_t telemetryHyperCall::ScatterReadVirtualMemory(void* descriptors_array,
                                                     std::uint64_t count,
                                                     std::uint64_t source_cr3)
{
    virt_memory_op_hypercall_info_t memory_op_call = {};
    memory_op_call.call_type = hypercall_type_t::_hc_0x121;
    memory_op_call.address_of_page_directory = source_cr3 >> 12;

    hypercall_info_t hypercall_info = {};
    hypercall_info.value = memory_op_call.value;
    const auto descriptors_va = reinterpret_cast<std::uint64_t>(descriptors_array);

    return MakeHypercall(hypercall_info.call_type,
                         hypercall_info.call_reserved_data,
                         descriptors_va,
                         count,
                         0);
}

std::uint64_t telemetryHyperCall::WriteGuestVirtualMemory(const void* const guest_source_buffer,
                                                     const std::uint64_t guest_destination_virtual_address,
                                                     const std::uint64_t destination_cr3,
                                                     const std::uint64_t size)
{
    virt_memory_op_hypercall_info_t memory_op_call = {};
    memory_op_call.call_type = hypercall_type_t::_hc_0x120;
    memory_op_call.memory_operation = memory_operation_t::write_operation;
    memory_op_call.address_of_page_directory = destination_cr3 >> 12;

    hypercall_info_t hypercall_info = {};
    hypercall_info.value = memory_op_call.value;
    const auto guest_source_virtual_address = reinterpret_cast<std::uint64_t>(guest_source_buffer);

    return MakeHypercall(hypercall_info.call_type,
                         hypercall_info.call_reserved_data,
                         guest_source_virtual_address,
                         guest_destination_virtual_address,
                         size);
}

std::uint64_t telemetryHyperCall::ReadGuestCr3()
{
    return MakeHypercall(hypercall_type_t::_hc_0x140, 0, 0, 0, 0);
}

std::uint64_t telemetryHyperCall::InjectMouseMovement(long x, long y, unsigned short flags)
{
    // New PS/2 Emulation Protocol:
    // RDX = dx, R8 = dy, R9 = buttons/flags
    return MakeHypercall(hypercall_type_t::_hc_0x220, 0,
                        static_cast<std::uint64_t>(x),
                        static_cast<std::uint64_t>(y),
                        static_cast<std::uint64_t>(flags));
}

bool telemetryHyperCall::SetMouseHookAddress(std::uint64_t addr)
{
    return MakeHypercall(hypercall_type_t::_hc_0x230, 0, addr, 0, 0) != 0;
}

bool telemetryHyperCall::ToggleProcessProtection(std::uint64_t eprocess_address, bool enable)
{
    return MakeHypercall(hypercall_type_t::_hc_0x250, 0, eprocess_address, static_cast<uint64_t>(enable), 0) == 1;
}

bool telemetryHyperCall::UnlinkProcess(std::uint64_t eprocess_address)
{
    return MakeHypercall(hypercall_type_t::_hc_0x260, 0, eprocess_address, 0, 0) == 1;
}

std::uint64_t telemetryHyperCall::GetHardwareFingerprint()
{
    return MakeHypercall(hypercall_type_t::_hc_0x270, 0, 0, 0, 0);
}

bool telemetryHyperCall::GetInputDiagnostics(input_diagnostics_snapshot_t* snapshot)
{
    if (snapshot == nullptr) {
        return false;
    }

    return MakeHypercall(hypercall_type_t::_hc_0x280, 0, reinterpret_cast<std::uint64_t>(snapshot), 0, 0) == 1;
}
