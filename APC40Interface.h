#pragma once

#include <map>
#include <vector>
#include <algorithm>

// ------------------------------------------------------------
/*

APC40 Interface Library v2

The concept of the library has changed drastically since version 1.

This interface is no longer responsible for sending midi messages.

Instead, it keeps track of the APC40's desired state and generates
midi messages on user request (APC40Interface::GetMidiMessages).

It is the user's responsibility to actually send them.
This is simple enough with libremidi, RtMidi and co.

The message buffer is optimized to only send changes.

Input is handled similarly, where the library just translates
raw midi input to APC40 specific messages.

NOTE:

When using libraries like libremidi or RtMidi on Windows, messages are
sent via WinMM's SendShortMsg. This function is extremely slow due to
throttling, so you should definitely keep an eye on that.

If you use raw WinMM on Windows you can use SendLongMsg instead and send
the whole midi buffer at once.

This allows rendering stuff on the APC at 120 fps or even higher.

Ideally, when on Windows, you should use the new winrt midi interface.

*/

// ------------------------------------------------------------ Definitions

constexpr int APC40_PAD_SIZE_X = 9;
constexpr int APC40_PAD_SIZE_Y = 10;

constexpr int APC40_NUM_SLIDERS = 9;
constexpr int APC40_NUM_KNOBS = 8;

enum class eAPC40Control
{
    MinValue = 0,

    Pad = MinValue,

    TrackPan = Pad + APC40_PAD_SIZE_X * APC40_PAD_SIZE_Y,
    TrackSendA,
    TrackSendB,
    TrackSendC,

    Shift,

    BankUp,
    BankDown,
    BankLeft,
    BankRight,

    TapTempo,
    NudgeDown,
    NudgeUp,

    DeviceClipTrack,
    DeviceToggle,
    DeviceLeft,
    DeviceRight,
    DeviceDetailView,
    DeviceRecQuantization,
    DeviceMidiOverdub,
    DeviceMetronome,

    Play,
    Stop,
    Rec,

    VolumeSlider,
    CrossfadeSlider = VolumeSlider + APC40_NUM_SLIDERS,
    CueLevelKnob,

    TrackKnobMode,
    TrackKnobValue = TrackKnobMode + APC40_NUM_KNOBS,

    DeviceKnobMode = TrackKnobValue + APC40_NUM_KNOBS,
    DeviceKnobValue = DeviceKnobMode + APC40_NUM_KNOBS,

    MaxValue = DeviceKnobValue + APC40_NUM_KNOBS,
    Invalid
};

enum class eAPC40LEDMode
{
    Off = 0,
    Green,
    GreenBlink,
    Red,
    RedBlink,
    Yellow,
    YellowBlink,
    On = 127
};

enum class eAPC40KnobMode
{
    Off = 0,
    Single,
    Volume,
    Pan
};

// Packs a base control value and an X/Y coordinate into a packed control value.
// Valid for eAPC40Control::Pad only.
constexpr eAPC40Control APC40PackControl(eAPC40Control control, int x, int y)
{
    if (control == eAPC40Control::Pad)
    {
        if (x < 0 || x >= APC40_PAD_SIZE_X || y < 0 || y >= APC40_PAD_SIZE_Y)
            return eAPC40Control::Invalid;

        return static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::Pad) + y * APC40_PAD_SIZE_X + x);
    }

    return eAPC40Control::Invalid;
}

// Packs a base control value and an id into a packed control value.
// Valid for VolumeSlider and knobs (Track/Device Mode/Value).
constexpr eAPC40Control APC40PackControl(eAPC40Control control, int id)
{
    if (control == eAPC40Control::VolumeSlider)
    {
        if (id < 0 || id >= APC40_NUM_SLIDERS)
            return eAPC40Control::Invalid;

        return static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::VolumeSlider) + id);
    }

    if (control == eAPC40Control::TrackKnobMode ||
        control == eAPC40Control::TrackKnobValue ||
        control == eAPC40Control::DeviceKnobMode ||
        control == eAPC40Control::DeviceKnobValue)
    {
        if (id < 0 || id >= APC40_NUM_KNOBS)
            return eAPC40Control::Invalid;

        return static_cast<eAPC40Control>(static_cast<int>(control) + id);
    }

    return eAPC40Control::Invalid;
}

// Unpacks the X coordinate from a packed control value.
constexpr int APC40UnpackControlX(eAPC40Control control)
{
    if (control >= eAPC40Control::Pad && control < static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::Pad) + APC40_PAD_SIZE_X * APC40_PAD_SIZE_Y))
    {
        return (static_cast<int>(control) - static_cast<int>(eAPC40Control::Pad)) % APC40_PAD_SIZE_X;
    }

    return 0;
}

// Unpacks the Y coordinate from a packed control value.
constexpr int APC40UnpackControlY(eAPC40Control control)
{
    if (control >= eAPC40Control::Pad && control < static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::Pad) + APC40_PAD_SIZE_X * APC40_PAD_SIZE_Y))
    {
        return (static_cast<int>(control) - static_cast<int>(eAPC40Control::Pad)) / APC40_PAD_SIZE_X;
    }

    return 0;
}

// Unpacks the id from a packed control value.
constexpr int APC40UnpackControlID(eAPC40Control control)
{
    if (control >= eAPC40Control::VolumeSlider && control < static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::VolumeSlider) + APC40_NUM_SLIDERS))
    {
        return static_cast<int>(control) - static_cast<int>(eAPC40Control::VolumeSlider);
    }

    if (control >= eAPC40Control::TrackKnobMode && control < static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::TrackKnobMode) + APC40_NUM_KNOBS))
    {
        return static_cast<int>(control) - static_cast<int>(eAPC40Control::TrackKnobMode);
    }

    if (control >= eAPC40Control::TrackKnobValue && control < static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::TrackKnobValue) + APC40_NUM_KNOBS))
    {
        return static_cast<int>(control) - static_cast<int>(eAPC40Control::TrackKnobValue);
    }

    if (control >= eAPC40Control::DeviceKnobMode && control < static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::DeviceKnobMode) + APC40_NUM_KNOBS))
    {
        return static_cast<int>(control) - static_cast<int>(eAPC40Control::DeviceKnobMode);
    }

    if (control >= eAPC40Control::DeviceKnobValue && control < static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::DeviceKnobValue) + APC40_NUM_KNOBS))
    {
        return static_cast<int>(control) - static_cast<int>(eAPC40Control::DeviceKnobValue);
    }

    return 0;
}

// Strips away X/Y coordinate or id from a packed control value.
constexpr eAPC40Control APC40StripControl(eAPC40Control control)
{
    if (control >= eAPC40Control::Pad && control < static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::Pad) + APC40_PAD_SIZE_X * APC40_PAD_SIZE_Y))
        return eAPC40Control::Pad;
    
    if (control >= eAPC40Control::VolumeSlider && control < static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::VolumeSlider) + APC40_NUM_SLIDERS))
        return eAPC40Control::VolumeSlider;
    
    if (control >= eAPC40Control::TrackKnobMode && control < static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::TrackKnobMode) + APC40_NUM_KNOBS))
        return eAPC40Control::TrackKnobMode;
    
    if (control >= eAPC40Control::TrackKnobValue && control < static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::TrackKnobValue) + APC40_NUM_KNOBS))
        return eAPC40Control::TrackKnobValue;

    if (control >= eAPC40Control::DeviceKnobMode && control < static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::DeviceKnobMode) + APC40_NUM_KNOBS))
        return eAPC40Control::DeviceKnobMode;

    if (control >= eAPC40Control::DeviceKnobValue && control < static_cast<eAPC40Control>(static_cast<int>(eAPC40Control::DeviceKnobValue) + APC40_NUM_KNOBS))
        return eAPC40Control::DeviceKnobValue;

    return control;
}

// ------------------------------------------------------------

struct APC40Input
{
    eAPC40Control control = eAPC40Control::Invalid;
    int value = 0;
    bool pressed = false;
};

// ------------------------------------------------------------ 

class APC40Interface
{
public:

    APC40Interface()
    {
        memset(m_CurrentState, 255, sizeof(m_CurrentState));
        memset(m_DesiredState, 0, sizeof(m_DesiredState));
    }

    ~APC40Interface()
    {

    }

    void GetInitMessage(std::vector<unsigned char>& message)
    {
        message =
        {
            0xF0, // MIDI excl start
            0x47, // Manufacturer ID
            0x7F, // Device ID
            0x73, // Product Model ID
            0x60, // Msg Type ID (0x60=Init)
            0x00, // Num Data Bytes (most sign.)
            0x04, // Num Data Bytes (least sign.)
            0x42, // Device Mode (0x40=unset, 0x41=Ableton, 0x42=Ableton with full ctrl)
            0x01, // PC Ver Major
            0x01, // PC Ver Minor
            0x01, // PC Bug Fix Lvl
            0xF7  // MIDI excl end
        };
    }

    // ------------------------------------------------------------ Output

    // Sets the mode of an APC40 control (leds, knob mode, etc).
    bool SetControlMode(eAPC40Control control, eAPC40LEDMode mode)
    {
        if (control < eAPC40Control::MinValue || control >= eAPC40Control::MaxValue)
            return false;

        m_DesiredState[static_cast<size_t>(control)] = static_cast<unsigned char>(std::clamp(static_cast<int>(mode), 0, 127));

        return true;
    }

    // Sets the mode of an APC40 control (leds, knob mode, etc).
    bool SetControlMode(eAPC40Control control, eAPC40KnobMode mode)
    {
        if (control < eAPC40Control::MinValue || control >= eAPC40Control::MaxValue)
            return false;

        m_DesiredState[static_cast<size_t>(control)] = static_cast<unsigned char>(std::clamp(static_cast<int>(mode), 0, 127));

        return true;
    }

    // Sets the raw value of an APC40 control (ie knob led count), 0 - 127.
    bool SetControlValue(eAPC40Control control, int value)
    {
        if (control < eAPC40Control::MinValue || control >= eAPC40Control::MaxValue)
            return false;

        m_DesiredState[static_cast<size_t>(control)] = static_cast<unsigned char>(std::clamp(value, 0, 127));

        return true;
    }

    // Gets the mode of an APC40 control
    bool GetControlMode(eAPC40Control control, eAPC40LEDMode& mode)
    {
        if (control < eAPC40Control::MinValue || control >= eAPC40Control::MaxValue)
            return false;

        mode = static_cast<eAPC40LEDMode>(m_DesiredState[static_cast<size_t>(control)]);

        return true;
    }

    // Gets the mode of an APC40 control
    bool GetControlMode(eAPC40Control control, eAPC40KnobMode& mode)
    {
        if (control < eAPC40Control::MinValue || control >= eAPC40Control::MaxValue)
            return false;

        mode = static_cast<eAPC40KnobMode>(m_DesiredState[static_cast<size_t>(control)]);

        return true;
    }

    // Get sthe raw value of an APC40 control
    bool GetControlValue(eAPC40Control control, int& value)
    {
        if (control < eAPC40Control::MinValue || control >= eAPC40Control::MaxValue)
            return false;

        value = static_cast<int>(m_DesiredState[static_cast<size_t>(control)]);

        return true;
    }

    // Should be called if the APC40 is disconnected.
    // That way, the library knows to send the midi messages required to restore the state on reconnect.
    void ResetCurrentState()
    {
        memset(m_CurrentState, 255, sizeof(m_CurrentState));
    }

    // Should be called if you actually want to reset all controls.
    void ResetDesiredState()
    {
        memset(m_DesiredState, 0, sizeof(m_DesiredState));
    }

    // Gets the Midi Message queue. Set update_state to false if you don't want to keep this state.
    // Midi messages are only generated on changed values (current device state vs. desired device state).
    // If running status is supported by the device (which depends on firmware version), you can save some bandwidth by enabling it.
    void GetMidiMessages(std::vector<unsigned char>& messages, bool update_state, bool running_status, unsigned int* num_messages = nullptr)
    {
        messages.clear();

        if (num_messages)
            *num_messages = 0;

        unsigned char b1_last{ 255 };

        for (size_t i = 0; i < static_cast<size_t>(eAPC40Control::MaxValue); ++i)
        {
            if (m_CurrentState[i] == m_DesiredState[i])
                continue;

            unsigned char b1;
            unsigned char b2;
            unsigned char b3;

            if (TranslateOutputMessage(static_cast<eAPC40Control>(i), m_DesiredState[i], b1, b2, b3))
            {
                if (running_status)
                {
                    if (b1 != b1_last)
                    {
                        messages.emplace_back(b1);
                        b1_last = b1;
                    }
                }
                else
                {
                    messages.emplace_back(b1);
                }

                messages.emplace_back(b2);
                messages.emplace_back(b3);

                if (num_messages)
                    ++(*num_messages);
            }
        }

        if (update_state)
            memcpy(m_CurrentState, m_DesiredState, static_cast<size_t>(eAPC40Control::MaxValue));
    }

    // ------------------------------------------------------------ Message Translation (Midi <-> APC)

    bool TranslateInputMessage(unsigned int midi_message, APC40Input& input_message)
    {
        unsigned char midi_message_arr[3]{ ((midi_message >> 0) & 0xFF), ((midi_message >> 8) & 0xFF), ((midi_message >> 16) & 0xFF) };

        return TranslateInputMessage(midi_message_arr, 3, input_message);
    }

    // midi_message is an array and always expected to be of size 3 or more (any indexes above 2 are ignored).
    bool TranslateInputMessage(unsigned char* midi_message, unsigned int midi_message_size, APC40Input& input_message)
    {
        if (midi_message_size < 3)
            return false;

        int value{ 0 };
        bool pressed{ true };

        int b1 = static_cast<int>(midi_message[0]);
        int b2 = static_cast<int>(midi_message[1]);
        int b3 = static_cast<int>(midi_message[2]);

        if (b1 >= 0x80 && b1 <= 0x8F)
        {
            b1 += 0x10;
            pressed = false;
        }

        auto it{ ms_ControlInputMap.find({ b1, b2 }) };

        if (it == ms_ControlInputMap.end())
            return false;

        input_message.control = static_cast<eAPC40Control>(it->second);
        input_message.value = std::clamp(b3, 0, 127);
        input_message.pressed = pressed;

        return true;
    }

    bool TranslateOutputMessage(eAPC40Control control, int value, unsigned char& b1, unsigned char& b2, unsigned char& b3)
    {
        auto it{ ms_ControlOutputMap.find(static_cast<int>(control)) };

        if (it == ms_ControlOutputMap.end())
            return false;

        b1 = static_cast<unsigned char>(it->second.first);
        b2 = static_cast<unsigned char>(it->second.second);
        b3 = static_cast<unsigned char>(std::clamp(value, 0, 127));

        return true;
    }

    // ------------------------------------------------------------ Utility

    // Gets an x, y coord for a position on the pad's circumference (0-31) - scene launch excluded
    bool GetPadCircularPos(int pos, int& x, int& y)
    {
        if (pos < 0 || pos >= 32)
            return false;

        if (pos >= 0 && pos <= 7) // Horizontal Top
        {
            x = pos;
            y = 0;
        }
        else if (pos >= 16 && pos <= 23) // Horizontal Bottom
        {
            x = 7 - (pos - 16);
            y = 9;
        }
        else if (pos >= 7 && pos <= 16) // Vertical Right
        {
            x = 7;
            y = pos - 7;
        }
        else if (pos >= 23 && pos <= 31) // Vertical Left
        {
            x = 0;
            y = 9 - (pos - 23);
        }

        return true;
    }

    bool IsValidPadPos(int x, int y, bool allow_scene = true)
    {
        if (allow_scene && x == 8 && y >= 0 && y < 7)
            return true;

        if (x < 0 || x > 7 || y < 0 || y > 9)
            return false;

        return true;
    }

    int GetKnobValueLEDCount(unsigned char value, eAPC40KnobMode mode)
    {
        switch (mode)
        {
        case eAPC40KnobMode::Single:
            if (value < 1) return 0;
            if (value < 10) return 1;
            if (value < 19) return 2;
            if (value < 28) return 3;
            if (value < 37) return 4;
            if (value < 46) return 5;
            if (value < 55) return 6;
            if (value < 64) return 7;
            if (value < 72) return 8;
            if (value < 81) return 9;
            if (value < 90) return 10;
            if (value < 99) return 11;
            if (value < 108) return 12;
            if (value < 117) return 13;
            if (value < 127) return 14;
            return 15;

        case eAPC40KnobMode::Volume:
            if (value < 1) return 0;
            if (value < 10) return 1;
            if (value < 19) return 2;
            if (value < 28) return 3;
            if (value < 37) return 4;
            if (value < 46) return 5;
            if (value < 55) return 6;
            if (value < 64) return 7;
            if (value < 72) return 8;
            if (value < 81) return 9;
            if (value < 90) return 10;
            if (value < 99) return 11;
            if (value < 108) return 12;
            if (value < 117) return 13;
            if (value < 127) return 14;
            return 15;

        case eAPC40KnobMode::Pan:
            if (value < 9) return -7;
            if (value < 18) return -6;
            if (value < 27) return -5;
            if (value < 36) return -4;
            if (value < 45) return -3;
            if (value < 54) return -2;
            if (value < 63) return -1;
            if (value < 65) return 0;
            if (value < 74) return 1;
            if (value < 83) return 2;
            if (value < 92) return 3;
            if (value < 101) return 4;
            if (value < 110) return 5;
            if (value < 119) return 6;
            return 7;
        }

        return 0;
    }

    // Only for Volume and Pan. Pan ranges from -7 to 7
    bool SetKnobValueLEDCount(eAPC40Control input, eAPC40KnobMode mode, int count)
    {
        constexpr int volume_values[16] = { 0, 5, 14, 23, 32, 41, 50, 59, 68, 76, 85, 94, 103, 112, 121, 127 };
        constexpr int pan_values[15] = { 4, 13, 22, 31, 40, 59, 58, 63, 69, 78, 87, 96, 105, 114, 123 };

        switch (mode)
        {
        case eAPC40KnobMode::Volume:

            SetControlMode(input, static_cast<eAPC40KnobMode>(volume_values[std::clamp(count, 0, 15)]));

            return true;

        case eAPC40KnobMode::Pan:

            SetControlMode(input, static_cast<eAPC40KnobMode>(pan_values[std::clamp(count + 7, 0, 15)]));

            return true;
        }

        return false;
    }

private:

    unsigned char m_CurrentState[static_cast<size_t>(eAPC40Control::MaxValue)];
    unsigned char m_DesiredState[static_cast<size_t>(eAPC40Control::MaxValue)];

    inline static std::map<std::pair<int, int>, int> ms_ControlInputMap = // TODO: Change the value (int) to eAPC40Control and make it readable
    {
        { std::make_pair(0x80, 0x30), 81 },
        { std::make_pair(0x80, 0x31), 72 },
        { std::make_pair(0x80, 0x32), 63 },
        { std::make_pair(0x80, 0x33), 54 },
        { std::make_pair(0x80, 0x34), 45 },
        { std::make_pair(0x80, 0x35), 0 },
        { std::make_pair(0x80, 0x36), 9 },
        { std::make_pair(0x80, 0x37), 18 },
        { std::make_pair(0x80, 0x38), 27 },
        { std::make_pair(0x80, 0x39), 36 },
        { std::make_pair(0x80, 0x3A), 102 },
        { std::make_pair(0x80, 0x3B), 103 },
        { std::make_pair(0x80, 0x3C), 104 },
        { std::make_pair(0x80, 0x3D), 105 },
        { std::make_pair(0x80, 0x3E), 106 },
        { std::make_pair(0x80, 0x3F), 107 },
        { std::make_pair(0x80, 0x40), 108 },
        { std::make_pair(0x80, 0x41), 109 },
        { std::make_pair(0x80, 0x50), 62 },
        { std::make_pair(0x80, 0x51), 53 },
        { std::make_pair(0x80, 0x52), 8 },
        { std::make_pair(0x80, 0x53), 17 },
        { std::make_pair(0x80, 0x54), 26 },
        { std::make_pair(0x80, 0x55), 35 },
        { std::make_pair(0x80, 0x56), 44 },
        { std::make_pair(0x80, 0x57), 90 },
        { std::make_pair(0x80, 0x58), 91 },
        { std::make_pair(0x80, 0x59), 92 },
        { std::make_pair(0x80, 0x5A), 93 },
        { std::make_pair(0x80, 0x5B), 110 },
        { std::make_pair(0x80, 0x5C), 111 },
        { std::make_pair(0x80, 0x5D), 112 },
        { std::make_pair(0x80, 0x5E), 95 },
        { std::make_pair(0x80, 0x5F), 96 },
        { std::make_pair(0x80, 0x60), 98 },
        { std::make_pair(0x80, 0x61), 97 },
        { std::make_pair(0x80, 0x62), 94 },
        { std::make_pair(0x80, 0x63), 99 },
        { std::make_pair(0x80, 0x64), 101 },
        { std::make_pair(0x80, 0x65), 100 },
        { std::make_pair(0x81, 0x30), 82 },
        { std::make_pair(0x81, 0x31), 73 },
        { std::make_pair(0x81, 0x32), 64 },
        { std::make_pair(0x81, 0x33), 55 },
        { std::make_pair(0x81, 0x34), 46 },
        { std::make_pair(0x81, 0x35), 1 },
        { std::make_pair(0x81, 0x36), 10 },
        { std::make_pair(0x81, 0x37), 19 },
        { std::make_pair(0x81, 0x38), 28 },
        { std::make_pair(0x81, 0x39), 37 },
        { std::make_pair(0x82, 0x30), 83 },
        { std::make_pair(0x82, 0x31), 74 },
        { std::make_pair(0x82, 0x32), 65 },
        { std::make_pair(0x82, 0x33), 56 },
        { std::make_pair(0x82, 0x34), 47 },
        { std::make_pair(0x82, 0x35), 2 },
        { std::make_pair(0x82, 0x36), 11 },
        { std::make_pair(0x82, 0x37), 20 },
        { std::make_pair(0x82, 0x38), 29 },
        { std::make_pair(0x82, 0x39), 38 },
        { std::make_pair(0x83, 0x30), 84 },
        { std::make_pair(0x83, 0x31), 75 },
        { std::make_pair(0x83, 0x32), 66 },
        { std::make_pair(0x83, 0x33), 57 },
        { std::make_pair(0x83, 0x34), 48 },
        { std::make_pair(0x83, 0x35), 3 },
        { std::make_pair(0x83, 0x36), 12 },
        { std::make_pair(0x83, 0x37), 21 },
        { std::make_pair(0x83, 0x38), 30 },
        { std::make_pair(0x83, 0x39), 39 },
        { std::make_pair(0x84, 0x30), 85 },
        { std::make_pair(0x84, 0x31), 76 },
        { std::make_pair(0x84, 0x32), 67 },
        { std::make_pair(0x84, 0x33), 58 },
        { std::make_pair(0x84, 0x34), 49 },
        { std::make_pair(0x84, 0x35), 4 },
        { std::make_pair(0x84, 0x36), 13 },
        { std::make_pair(0x84, 0x37), 22 },
        { std::make_pair(0x84, 0x38), 31 },
        { std::make_pair(0x84, 0x39), 40 },
        { std::make_pair(0x85, 0x30), 86 },
        { std::make_pair(0x85, 0x31), 77 },
        { std::make_pair(0x85, 0x32), 68 },
        { std::make_pair(0x85, 0x33), 59 },
        { std::make_pair(0x85, 0x34), 50 },
        { std::make_pair(0x85, 0x35), 5 },
        { std::make_pair(0x85, 0x36), 14 },
        { std::make_pair(0x85, 0x37), 23 },
        { std::make_pair(0x85, 0x38), 32 },
        { std::make_pair(0x85, 0x39), 41 },
        { std::make_pair(0x86, 0x30), 87 },
        { std::make_pair(0x86, 0x31), 78 },
        { std::make_pair(0x86, 0x32), 69 },
        { std::make_pair(0x86, 0x33), 60 },
        { std::make_pair(0x86, 0x34), 51 },
        { std::make_pair(0x86, 0x35), 6 },
        { std::make_pair(0x86, 0x36), 15 },
        { std::make_pair(0x86, 0x37), 24 },
        { std::make_pair(0x86, 0x38), 33 },
        { std::make_pair(0x86, 0x39), 42 },
        { std::make_pair(0x87, 0x30), 88 },
        { std::make_pair(0x87, 0x31), 79 },
        { std::make_pair(0x87, 0x32), 70 },
        { std::make_pair(0x87, 0x33), 61 },
        { std::make_pair(0x87, 0x34), 52 },
        { std::make_pair(0x87, 0x35), 7 },
        { std::make_pair(0x87, 0x36), 16 },
        { std::make_pair(0x87, 0x37), 25 },
        { std::make_pair(0x87, 0x38), 34 },
        { std::make_pair(0x87, 0x39), 43 },
        { std::make_pair(0x90, 0x30), 81 },
        { std::make_pair(0x90, 0x31), 72 },
        { std::make_pair(0x90, 0x32), 63 },
        { std::make_pair(0x90, 0x33), 54 },
        { std::make_pair(0x90, 0x34), 45 },
        { std::make_pair(0x90, 0x35), 0 },
        { std::make_pair(0x90, 0x36), 9 },
        { std::make_pair(0x90, 0x37), 18 },
        { std::make_pair(0x90, 0x38), 27 },
        { std::make_pair(0x90, 0x39), 36 },
        { std::make_pair(0x90, 0x3A), 102 },
        { std::make_pair(0x90, 0x3B), 103 },
        { std::make_pair(0x90, 0x3C), 104 },
        { std::make_pair(0x90, 0x3D), 105 },
        { std::make_pair(0x90, 0x3E), 106 },
        { std::make_pair(0x90, 0x3F), 107 },
        { std::make_pair(0x90, 0x40), 108 },
        { std::make_pair(0x90, 0x41), 109 },
        { std::make_pair(0x90, 0x50), 62 },
        { std::make_pair(0x90, 0x51), 53 },
        { std::make_pair(0x90, 0x52), 8 },
        { std::make_pair(0x90, 0x53), 17 },
        { std::make_pair(0x90, 0x54), 26 },
        { std::make_pair(0x90, 0x55), 35 },
        { std::make_pair(0x90, 0x56), 44 },
        { std::make_pair(0x90, 0x57), 90 },
        { std::make_pair(0x90, 0x58), 91 },
        { std::make_pair(0x90, 0x59), 92 },
        { std::make_pair(0x90, 0x5A), 93 },
        { std::make_pair(0x90, 0x5B), 110 },
        { std::make_pair(0x90, 0x5C), 111 },
        { std::make_pair(0x90, 0x5D), 112 },
        { std::make_pair(0x90, 0x5E), 95 },
        { std::make_pair(0x90, 0x5F), 96 },
        { std::make_pair(0x90, 0x60), 98 },
        { std::make_pair(0x90, 0x61), 97 },
        { std::make_pair(0x90, 0x62), 94 },
        { std::make_pair(0x90, 0x63), 99 },
        { std::make_pair(0x90, 0x64), 101 },
        { std::make_pair(0x90, 0x65), 100 },
        { std::make_pair(0x91, 0x30), 82 },
        { std::make_pair(0x91, 0x31), 73 },
        { std::make_pair(0x91, 0x32), 64 },
        { std::make_pair(0x91, 0x33), 55 },
        { std::make_pair(0x91, 0x34), 46 },
        { std::make_pair(0x91, 0x35), 1 },
        { std::make_pair(0x91, 0x36), 10 },
        { std::make_pair(0x91, 0x37), 19 },
        { std::make_pair(0x91, 0x38), 28 },
        { std::make_pair(0x91, 0x39), 37 },
        { std::make_pair(0x92, 0x30), 83 },
        { std::make_pair(0x92, 0x31), 74 },
        { std::make_pair(0x92, 0x32), 65 },
        { std::make_pair(0x92, 0x33), 56 },
        { std::make_pair(0x92, 0x34), 47 },
        { std::make_pair(0x92, 0x35), 2 },
        { std::make_pair(0x92, 0x36), 11 },
        { std::make_pair(0x92, 0x37), 20 },
        { std::make_pair(0x92, 0x38), 29 },
        { std::make_pair(0x92, 0x39), 38 },
        { std::make_pair(0x93, 0x30), 84 },
        { std::make_pair(0x93, 0x31), 75 },
        { std::make_pair(0x93, 0x32), 66 },
        { std::make_pair(0x93, 0x33), 57 },
        { std::make_pair(0x93, 0x34), 48 },
        { std::make_pair(0x93, 0x35), 3 },
        { std::make_pair(0x93, 0x36), 12 },
        { std::make_pair(0x93, 0x37), 21 },
        { std::make_pair(0x93, 0x38), 30 },
        { std::make_pair(0x93, 0x39), 39 },
        { std::make_pair(0x94, 0x30), 85 },
        { std::make_pair(0x94, 0x31), 76 },
        { std::make_pair(0x94, 0x32), 67 },
        { std::make_pair(0x94, 0x33), 58 },
        { std::make_pair(0x94, 0x34), 49 },
        { std::make_pair(0x94, 0x35), 4 },
        { std::make_pair(0x94, 0x36), 13 },
        { std::make_pair(0x94, 0x37), 22 },
        { std::make_pair(0x94, 0x38), 31 },
        { std::make_pair(0x94, 0x39), 40 },
        { std::make_pair(0x95, 0x30), 86 },
        { std::make_pair(0x95, 0x31), 77 },
        { std::make_pair(0x95, 0x32), 68 },
        { std::make_pair(0x95, 0x33), 59 },
        { std::make_pair(0x95, 0x34), 50 },
        { std::make_pair(0x95, 0x35), 5 },
        { std::make_pair(0x95, 0x36), 14 },
        { std::make_pair(0x95, 0x37), 23 },
        { std::make_pair(0x95, 0x38), 32 },
        { std::make_pair(0x95, 0x39), 41 },
        { std::make_pair(0x96, 0x30), 87 },
        { std::make_pair(0x96, 0x31), 78 },
        { std::make_pair(0x96, 0x32), 69 },
        { std::make_pair(0x96, 0x33), 60 },
        { std::make_pair(0x96, 0x34), 51 },
        { std::make_pair(0x96, 0x35), 6 },
        { std::make_pair(0x96, 0x36), 15 },
        { std::make_pair(0x96, 0x37), 24 },
        { std::make_pair(0x96, 0x38), 33 },
        { std::make_pair(0x96, 0x39), 42 },
        { std::make_pair(0x97, 0x30), 88 },
        { std::make_pair(0x97, 0x31), 79 },
        { std::make_pair(0x97, 0x32), 70 },
        { std::make_pair(0x97, 0x33), 61 },
        { std::make_pair(0x97, 0x34), 52 },
        { std::make_pair(0x97, 0x35), 7 },
        { std::make_pair(0x97, 0x36), 16 },
        { std::make_pair(0x97, 0x37), 25 },
        { std::make_pair(0x97, 0x38), 34 },
        { std::make_pair(0x97, 0x39), 43 },
        { std::make_pair(0xB0, 0x07), 113 },
        { std::make_pair(0xB0, 0x0E), 121 },
        { std::make_pair(0xB0, 0x0F), 122 },
        { std::make_pair(0xB0, 0x10), 148 },
        { std::make_pair(0xB0, 0x11), 149 },
        { std::make_pair(0xB0, 0x12), 150 },
        { std::make_pair(0xB0, 0x13), 151 },
        { std::make_pair(0xB0, 0x14), 152 },
        { std::make_pair(0xB0, 0x15), 153 },
        { std::make_pair(0xB0, 0x16), 154 },
        { std::make_pair(0xB0, 0x17), 155 },
        { std::make_pair(0xB0, 0x2F), 123 },
        { std::make_pair(0xB0, 0x30), 132 },
        { std::make_pair(0xB0, 0x31), 133 },
        { std::make_pair(0xB0, 0x32), 134 },
        { std::make_pair(0xB0, 0x33), 135 },
        { std::make_pair(0xB0, 0x34), 136 },
        { std::make_pair(0xB0, 0x35), 137 },
        { std::make_pair(0xB0, 0x36), 138 },
        { std::make_pair(0xB0, 0x37), 139 },
        { std::make_pair(0xB1, 0x07), 114 },
        { std::make_pair(0xB2, 0x07), 115 },
        { std::make_pair(0xB3, 0x07), 116 },
        { std::make_pair(0xB4, 0x07), 117 },
        { std::make_pair(0xB5, 0x07), 118 },
        { std::make_pair(0xB6, 0x07), 119 },
        { std::make_pair(0xB7, 0x07), 120 },
    };

    inline static std::map<int, std::pair<int, int>> ms_ControlOutputMap = // TODO: Change the key (int) to eAPC40Control and make it readable
    {
        { 0, std::make_pair(0x90, 0x35) },
        { 1, std::make_pair(0x91, 0x35) },
        { 2, std::make_pair(0x92, 0x35) },
        { 3, std::make_pair(0x93, 0x35) },
        { 4, std::make_pair(0x94, 0x35) },
        { 5, std::make_pair(0x95, 0x35) },
        { 6, std::make_pair(0x96, 0x35) },
        { 7, std::make_pair(0x97, 0x35) },
        { 8, std::make_pair(0x90, 0x52) },
        { 9, std::make_pair(0x90, 0x36) },
        { 10, std::make_pair(0x91, 0x36) },
        { 11, std::make_pair(0x92, 0x36) },
        { 12, std::make_pair(0x93, 0x36) },
        { 13, std::make_pair(0x94, 0x36) },
        { 14, std::make_pair(0x95, 0x36) },
        { 15, std::make_pair(0x96, 0x36) },
        { 16, std::make_pair(0x97, 0x36) },
        { 17, std::make_pair(0x90, 0x53) },
        { 18, std::make_pair(0x90, 0x37) },
        { 19, std::make_pair(0x91, 0x37) },
        { 20, std::make_pair(0x92, 0x37) },
        { 21, std::make_pair(0x93, 0x37) },
        { 22, std::make_pair(0x94, 0x37) },
        { 23, std::make_pair(0x95, 0x37) },
        { 24, std::make_pair(0x96, 0x37) },
        { 25, std::make_pair(0x97, 0x37) },
        { 26, std::make_pair(0x90, 0x54) },
        { 27, std::make_pair(0x90, 0x38) },
        { 28, std::make_pair(0x91, 0x38) },
        { 29, std::make_pair(0x92, 0x38) },
        { 30, std::make_pair(0x93, 0x38) },
        { 31, std::make_pair(0x94, 0x38) },
        { 32, std::make_pair(0x95, 0x38) },
        { 33, std::make_pair(0x96, 0x38) },
        { 34, std::make_pair(0x97, 0x38) },
        { 35, std::make_pair(0x90, 0x55) },
        { 36, std::make_pair(0x90, 0x39) },
        { 37, std::make_pair(0x91, 0x39) },
        { 38, std::make_pair(0x92, 0x39) },
        { 39, std::make_pair(0x93, 0x39) },
        { 40, std::make_pair(0x94, 0x39) },
        { 41, std::make_pair(0x95, 0x39) },
        { 42, std::make_pair(0x96, 0x39) },
        { 43, std::make_pair(0x97, 0x39) },
        { 44, std::make_pair(0x90, 0x56) },
        { 45, std::make_pair(0x90, 0x34) },
        { 46, std::make_pair(0x91, 0x34) },
        { 47, std::make_pair(0x92, 0x34) },
        { 48, std::make_pair(0x93, 0x34) },
        { 49, std::make_pair(0x94, 0x34) },
        { 50, std::make_pair(0x95, 0x34) },
        { 51, std::make_pair(0x96, 0x34) },
        { 52, std::make_pair(0x97, 0x34) },
        { 53, std::make_pair(0x90, 0x51) },
        { 54, std::make_pair(0x90, 0x33) },
        { 55, std::make_pair(0x91, 0x33) },
        { 56, std::make_pair(0x92, 0x33) },
        { 57, std::make_pair(0x93, 0x33) },
        { 58, std::make_pair(0x94, 0x33) },
        { 59, std::make_pair(0x95, 0x33) },
        { 60, std::make_pair(0x96, 0x33) },
        { 61, std::make_pair(0x97, 0x33) },
        { 62, std::make_pair(0x90, 0x50) },
        { 63, std::make_pair(0x90, 0x32) },
        { 64, std::make_pair(0x91, 0x32) },
        { 65, std::make_pair(0x92, 0x32) },
        { 66, std::make_pair(0x93, 0x32) },
        { 67, std::make_pair(0x94, 0x32) },
        { 68, std::make_pair(0x95, 0x32) },
        { 69, std::make_pair(0x96, 0x32) },
        { 70, std::make_pair(0x97, 0x32) },
        { 72, std::make_pair(0x90, 0x31) },
        { 73, std::make_pair(0x91, 0x31) },
        { 74, std::make_pair(0x92, 0x31) },
        { 75, std::make_pair(0x93, 0x31) },
        { 76, std::make_pair(0x94, 0x31) },
        { 77, std::make_pair(0x95, 0x31) },
        { 78, std::make_pair(0x96, 0x31) },
        { 79, std::make_pair(0x97, 0x31) },
        { 81, std::make_pair(0x90, 0x30) },
        { 82, std::make_pair(0x91, 0x30) },
        { 83, std::make_pair(0x92, 0x30) },
        { 84, std::make_pair(0x93, 0x30) },
        { 85, std::make_pair(0x94, 0x30) },
        { 86, std::make_pair(0x95, 0x30) },
        { 87, std::make_pair(0x96, 0x30) },
        { 88, std::make_pair(0x97, 0x30) },
        { 90, std::make_pair(0x90, 0x57) },
        { 91, std::make_pair(0x90, 0x58) },
        { 92, std::make_pair(0x90, 0x59) },
        { 93, std::make_pair(0x90, 0x5A) },
        { 102, std::make_pair(0x90, 0x3A) },
        { 103, std::make_pair(0x90, 0x3B) },
        { 104, std::make_pair(0x90, 0x3C) },
        { 105, std::make_pair(0x90, 0x3D) },
        { 106, std::make_pair(0x90, 0x3E) },
        { 107, std::make_pair(0x90, 0x3F) },
        { 108, std::make_pair(0x90, 0x40) },
        { 109, std::make_pair(0x90, 0x41) },
        { 124, std::make_pair(0xB0, 0x38) },
        { 125, std::make_pair(0xB0, 0x39) },
        { 126, std::make_pair(0xB0, 0x3A) },
        { 127, std::make_pair(0xB0, 0x3B) },
        { 128, std::make_pair(0xB0, 0x3C) },
        { 129, std::make_pair(0xB0, 0x3D) },
        { 130, std::make_pair(0xB0, 0x3E) },
        { 131, std::make_pair(0xB0, 0x3F) },
        { 132, std::make_pair(0xB0, 0x30) },
        { 133, std::make_pair(0xB0, 0x31) },
        { 134, std::make_pair(0xB0, 0x32) },
        { 135, std::make_pair(0xB0, 0x33) },
        { 136, std::make_pair(0xB0, 0x34) },
        { 137, std::make_pair(0xB0, 0x35) },
        { 138, std::make_pair(0xB0, 0x36) },
        { 139, std::make_pair(0xB0, 0x37) },
        { 140, std::make_pair(0xB0, 0x18) },
        { 141, std::make_pair(0xB0, 0x19) },
        { 142, std::make_pair(0xB0, 0x1A) },
        { 143, std::make_pair(0xB0, 0x1B) },
        { 144, std::make_pair(0xB0, 0x1C) },
        { 145, std::make_pair(0xB0, 0x1D) },
        { 146, std::make_pair(0xB0, 0x1E) },
        { 147, std::make_pair(0xB0, 0x1F) },
        { 148, std::make_pair(0xB0, 0x10) },
        { 149, std::make_pair(0xB0, 0x11) },
        { 150, std::make_pair(0xB0, 0x12) },
        { 151, std::make_pair(0xB0, 0x13) },
        { 152, std::make_pair(0xB0, 0x14) },
        { 153, std::make_pair(0xB0, 0x15) },
        { 154, std::make_pair(0xB0, 0x16) },
        { 155, std::make_pair(0xB0, 0x17) },
    };
};

// ------------------------------------------------------------ EOF