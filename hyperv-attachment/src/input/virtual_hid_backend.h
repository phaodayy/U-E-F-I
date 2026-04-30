#pragma once

#include "input_backend.h"

namespace input {
    class VirtualHidBackend final : public InputBackend {
    public:
        BackendKind kind() const override;
        const char* name() const override;
        void initialize() override;
        bool is_available() const override;
        bool activate() override;
        bool inject_mouse(const MouseMovement& movement) override;
        bool handle_io(IoOperation& operation) override;
    };
}
