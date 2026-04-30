#include "mouse_ps2.h"
#include "../crt/crt.h"
#include "../input/input_diagnostics.h"
#include <intrin.h>

namespace arch::mouse_ps2 {
    MouseState g_mouse_state = { 0 };

    namespace {
        constexpr uint8_t kCommandByteInterruptKeyboard = 0x01;
        constexpr uint8_t kCommandByteInterruptMouse = 0x02;
        constexpr uint8_t kCommandByteSystemFlag = 0x04;
        constexpr uint8_t kCommandByteDisableKeyboard = 0x10;
        constexpr uint8_t kCommandByteDisableMouse = 0x20;
        constexpr uint8_t kCommandByteTranslation = 0x40;
        constexpr uint64_t kOutputLifetimeTsc = 1000ULL * 1000ULL * 1000ULL;
        constexpr uint32_t kControllerWaitIterations = 0x4000;
        constexpr uint8_t kControllerWriteMouseOutput = 0xD3;

        constexpr int32_t ClampPs2Delta(const int32_t value)
        {
            if (value > 127) return 127;
            if (value < -127) return -127;
            return value;
        }

        bool PushOutput(const uint8_t data, const bool mouse_data)
        {
            if (g_mouse_state.count >= static_cast<uint8_t>(sizeof(g_mouse_state.queue) / sizeof(g_mouse_state.queue[0]))) {
                return false;
            }

            g_mouse_state.queue[g_mouse_state.tail] = { data, mouse_data };
            g_mouse_state.tail = static_cast<uint8_t>((g_mouse_state.tail + 1) % (sizeof(g_mouse_state.queue) / sizeof(g_mouse_state.queue[0])));
            ++g_mouse_state.count;
            g_mouse_state.output_expire_tsc = __rdtsc() + kOutputLifetimeTsc;
            input::diagnostics::Increment(input::diagnostics::Counter::Ps2OutputQueued);
            return true;
        }

        void ClearOutput()
        {
            g_mouse_state.head = 0;
            g_mouse_state.tail = 0;
            g_mouse_state.count = 0;
            g_mouse_state.output_expire_tsc = 0;
            g_mouse_state.pending_write = PendingWrite::None;
        }

        void DropExpiredOutput()
        {
            if (g_mouse_state.count == 0 || g_mouse_state.output_expire_tsc == 0) {
                return;
            }

            if (static_cast<int64_t>(__rdtsc() - g_mouse_state.output_expire_tsc) >= 0) {
                ClearOutput();
                input::diagnostics::Increment(input::diagnostics::Counter::Ps2OutputExpired);
            }
        }

        bool PopOutput(OutputByte* const output)
        {
            if (g_mouse_state.count == 0 || output == nullptr) {
                return false;
            }

            *output = g_mouse_state.queue[g_mouse_state.head];
            g_mouse_state.head = static_cast<uint8_t>((g_mouse_state.head + 1) % (sizeof(g_mouse_state.queue) / sizeof(g_mouse_state.queue[0])));
            --g_mouse_state.count;
            input::diagnostics::Increment(input::diagnostics::Counter::Ps2OutputPopped);
            return true;
        }

        void QueueAck(const bool mouse_data)
        {
            (void)PushOutput(PS2_ACK, mouse_data);
            input::diagnostics::Increment(input::diagnostics::Counter::Ps2AckQueued);
        }

        void QueueMousePacket(int32_t dx, int32_t dy, const uint8_t buttons)
        {
            dx = ClampPs2Delta(dx);
            dy = ClampPs2Delta(dy);

            uint8_t first = 0x08 | (buttons & 0x07);
            if (dx < 0) first |= 0x10;
            if (dy < 0) first |= 0x20;

            (void)PushOutput(first, true);
            (void)PushOutput(static_cast<uint8_t>(dx & 0xFF), true);
            (void)PushOutput(static_cast<uint8_t>(dy & 0xFF), true);
            input::diagnostics::Increment(input::diagnostics::Counter::Ps2MovementPacket);
        }

        bool WaitInputClear()
        {
            for (uint32_t i = 0; i < kControllerWaitIterations; ++i) {
                if ((__inbyte(PS2_STATUS_PORT) & PS2_STATUS_INPUT_BUFFER_FULL) == 0) {
                    return true;
                }
                _mm_pause();
            }

            return false;
        }

        bool WaitOutputClear()
        {
            for (uint32_t i = 0; i < kControllerWaitIterations; ++i) {
                if ((__inbyte(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_BUFFER_FULL) == 0) {
                    return true;
                }
                _mm_pause();
            }

            return false;
        }

        bool WriteMouseOutputByte(const uint8_t data)
        {
            if (!WaitOutputClear() || !WaitInputClear()) {
                return false;
            }

            __outbyte(PS2_STATUS_PORT, kControllerWriteMouseOutput);

            if (!WaitInputClear()) {
                return false;
            }

            __outbyte(PS2_DATA_PORT, data);
            return true;
        }

        bool InjectPacketViaController(int32_t dx, int32_t dy, const uint8_t buttons)
        {
            dx = ClampPs2Delta(dx);
            dy = ClampPs2Delta(dy);

            uint8_t first = 0x08 | (buttons & 0x07);
            if (dx < 0) first |= 0x10;
            if (dy < 0) first |= 0x20;

            return WriteMouseOutputByte(first) &&
                WriteMouseOutputByte(static_cast<uint8_t>(dx & 0xFF)) &&
                WriteMouseOutputByte(static_cast<uint8_t>(dy & 0xFF));
        }

        void HandleMouseCommand(const uint8_t command)
        {
            QueueAck(true);

            switch (command) {
            case 0xFF: // Reset
                g_mouse_state.mouse_enabled = false;
                (void)PushOutput(0xAA, true);
                (void)PushOutput(0x00, true);
                break;
            case 0xF4: // Enable data reporting
                g_mouse_state.mouse_enabled = true;
                break;
            case 0xF5: // Disable data reporting
                g_mouse_state.mouse_enabled = false;
                break;
            case 0xF2: // Identify
                (void)PushOutput(0x00, true);
                break;
            case 0xEB: // Read data
                QueueMousePacket(0, 0, g_mouse_state.mouse_buttons);
                break;
            case 0xE9: // Status request
                (void)PushOutput(g_mouse_state.mouse_enabled ? 0x20 : 0x00, true);
                (void)PushOutput(0x02, true);
                (void)PushOutput(100, true);
                break;
            case 0xF3: // Set sample rate
                g_mouse_state.pending_write = PendingWrite::MouseSampleRate;
                break;
            case 0xE8: // Set resolution
                g_mouse_state.pending_write = PendingWrite::MouseResolution;
                break;
            case 0xEC: // Reset wrap mode
            case 0xEE: // Set wrap mode
                break;
            default:
                break;
            }
        }

        void HandleKeyboardCommand(const uint8_t command)
        {
            switch (command) {
            case 0xED: // Set LEDs
                QueueAck(false);
                g_mouse_state.pending_write = PendingWrite::KeyboardLed;
                break;
            case 0xF3: // Set typematic
                QueueAck(false);
                g_mouse_state.pending_write = PendingWrite::KeyboardTypematic;
                break;
            case 0xFF: // Reset
                QueueAck(false);
                (void)PushOutput(0xAA, false);
                break;
            case 0xF2: // Identify
                QueueAck(false);
                (void)PushOutput(0xAB, false);
                (void)PushOutput(0x83, false);
                break;
            case 0xF4: // Enable scanning
                g_mouse_state.keyboard_enabled = true;
                QueueAck(false);
                break;
            case 0xF5: // Disable scanning
                g_mouse_state.keyboard_enabled = false;
                QueueAck(false);
                break;
            default:
                QueueAck(false);
                break;
            }
        }
    }

    void Initialize()
    {
        crt::set_memory(&g_mouse_state, 0, sizeof(g_mouse_state));
        g_mouse_state.command_byte =
            kCommandByteInterruptKeyboard |
            kCommandByteInterruptMouse |
            kCommandByteSystemFlag |
            kCommandByteTranslation;
        g_mouse_state.mouse_enabled = true;
        g_mouse_state.keyboard_enabled = true;
        g_mouse_state.aux_enabled = true;
        g_mouse_state.pending_write = PendingWrite::None;
    }

    bool PushMovement(int32_t dx, int32_t dy, const uint8_t buttons)
    {
        bool queued = false;
        g_mouse_state.mouse_buttons = buttons & 0x07;

        while (dx != 0 || dy != 0) {
            const int32_t step_x = ClampPs2Delta(dx);
            const int32_t step_y = ClampPs2Delta(dy);

            if ((sizeof(g_mouse_state.queue) / sizeof(g_mouse_state.queue[0])) - g_mouse_state.count < 3) {
                break;
            }

            QueueMousePacket(step_x, step_y, g_mouse_state.mouse_buttons);
            queued = true;
            dx -= step_x;
            dy -= step_y;
        }

        return queued;
    }

    bool InjectMovementViaController(int32_t dx, int32_t dy, const uint8_t buttons)
    {
        return PushMovement(dx, dy, buttons);
    }

    bool HasPendingOutput()
    {
        DropExpiredOutput();
        return g_mouse_state.count != 0;
    }

    uint8_t HandleReadData()
    {
        input::diagnostics::Increment(input::diagnostics::Counter::Ps2DataRead);
        DropExpiredOutput();

        OutputByte output = {};
        if (!PopOutput(&output)) {
            return 0;
        }

        return output.data;
    }

    uint8_t HandleReadStatus()
    {
        input::diagnostics::Increment(input::diagnostics::Counter::Ps2StatusRead);
        DropExpiredOutput();

        uint8_t status = PS2_STATUS_SYSTEM_FLAG;

        if (g_mouse_state.count != 0) {
            status |= PS2_STATUS_OUTPUT_BUFFER_FULL;
            if (g_mouse_state.queue[g_mouse_state.head].mouse_data) {
                status |= PS2_STATUS_MOUSE_DATA;
            }
        }

        return status;
    }

    bool HandleWriteData(const uint8_t data)
    {
        input::diagnostics::Increment(input::diagnostics::Counter::Ps2DataWrite);

        switch (g_mouse_state.pending_write) {
        case PendingWrite::ControllerCommandByte:
            g_mouse_state.command_byte = data;
            g_mouse_state.keyboard_enabled = (data & kCommandByteDisableKeyboard) == 0;
            g_mouse_state.aux_enabled = (data & kCommandByteDisableMouse) == 0;
            g_mouse_state.pending_write = PendingWrite::None;
            return true;
        case PendingWrite::MouseCommand:
            g_mouse_state.pending_write = PendingWrite::None;
            HandleMouseCommand(data);
            return true;
        case PendingWrite::MouseSampleRate:
        case PendingWrite::MouseResolution:
        case PendingWrite::MouseWrapMode:
            QueueAck(true);
            g_mouse_state.pending_write = PendingWrite::None;
            return true;
        case PendingWrite::KeyboardLed:
        case PendingWrite::KeyboardTypematic:
            QueueAck(false);
            g_mouse_state.pending_write = PendingWrite::None;
            return true;
        case PendingWrite::None:
        default:
            break;
        }

        return false;
    }

    bool HandleWriteCommand(const uint8_t command)
    {
        input::diagnostics::Increment(input::diagnostics::Counter::Ps2StatusWrite);

        switch (command) {
        case 0x20: // Read controller command byte
            (void)PushOutput(g_mouse_state.command_byte, false);
            return true;
        case 0x60: // Write controller command byte
            g_mouse_state.pending_write = PendingWrite::ControllerCommandByte;
            return true;
        case 0xAA: // Controller self-test
            (void)PushOutput(0x55, false);
            return true;
        case 0xA9: // Mouse interface test
            (void)PushOutput(0x00, false);
            return true;
        case 0xA7: // Disable mouse port
            g_mouse_state.aux_enabled = false;
            g_mouse_state.command_byte |= kCommandByteDisableMouse;
            return true;
        case 0xA8: // Enable mouse port
            g_mouse_state.aux_enabled = true;
            g_mouse_state.command_byte &= ~kCommandByteDisableMouse;
            return true;
        case 0xD4: // Next data byte is a mouse command
            g_mouse_state.pending_write = PendingWrite::MouseCommand;
            return true;
        default:
            break;
        }

        return false;
    }
}
