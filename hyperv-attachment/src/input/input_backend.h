#pragma once

#include <cstdint>

namespace input {
    enum class BackendKind : std::uint64_t {
        None = 0,
        Ps2 = 1,
        VirtualHid = 2,
        Diagnostics = 3
    };

    enum BackendMask : std::uint64_t {
        BackendMaskNone = 0,
        BackendMaskPs2 = 1ULL << 0,
        BackendMaskVirtualHid = 1ULL << 1,
        BackendMaskDiagnostics = 1ULL << 2
    };

    struct MouseMovement {
        std::int32_t dx;
        std::int32_t dy;
        std::uint8_t buttons;
    };

    struct IoOperation {
        std::uint16_t port;
        std::uint8_t is_in;
        std::uint8_t is_byte_access;
        std::uint8_t is_string;
        std::uint8_t is_rep;
        std::uint8_t al;
        std::uint8_t result_al;
    };

    class InputBackend {
    public:
        virtual BackendKind kind() const = 0;
        virtual const char* name() const = 0;
        virtual void initialize() = 0;
        virtual bool is_available() const = 0;
        virtual bool activate() = 0;
        virtual bool inject_mouse(const MouseMovement& movement) = 0;
        virtual bool handle_io(IoOperation& operation) = 0;
    };
}
