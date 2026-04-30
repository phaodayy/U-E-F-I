#pragma once
#include <cstdint>

namespace arch::mouse_ps2 {
    enum class PendingWrite : uint8_t {
        None,
        ControllerCommandByte,
        MouseCommand,
        MouseSampleRate,
        MouseResolution,
        MouseWrapMode,
        KeyboardLed,
        KeyboardTypematic
    };

    struct OutputByte {
        uint8_t data;
        bool mouse_data;
    };

    struct MouseState {
        OutputByte queue[128];
        uint8_t head;
        uint8_t tail;
        uint8_t count;
        uint8_t command_byte;
        uint8_t mouse_buttons;
        uint64_t output_expire_tsc;
        bool mouse_enabled;
        bool keyboard_enabled;
        bool aux_enabled;
        PendingWrite pending_write;
    };

    extern MouseState g_mouse_state;

    constexpr uint16_t PS2_DATA_PORT = 0x60;
    constexpr uint16_t PS2_STATUS_PORT = 0x64;

    constexpr uint8_t PS2_STATUS_OUTPUT_BUFFER_FULL = 0x01;
    constexpr uint8_t PS2_STATUS_INPUT_BUFFER_FULL  = 0x02;
    constexpr uint8_t PS2_STATUS_SYSTEM_FLAG        = 0x04;
    constexpr uint8_t PS2_STATUS_MOUSE_DATA         = 0x20;

    constexpr uint8_t PS2_ACK = 0xFA;
    constexpr uint8_t PS2_RESEND = 0xFE;

    void Initialize();
    bool PushMovement(int32_t dx, int32_t dy, uint8_t buttons);
    bool InjectMovementViaController(int32_t dx, int32_t dy, uint8_t buttons);
    bool HasPendingOutput();
    uint8_t HandleReadData();
    uint8_t HandleReadStatus();
    bool HandleWriteData(uint8_t data);
    bool HandleWriteCommand(uint8_t command);
}
