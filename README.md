# APC40Interface

C++ Library to interact with the Akai APC 40 (portable MIDI controller). There is great support for Ableton, Tractor, FL Studio etc. so this is not meant for those. This is to play around with it without digging into the protocols.

Uses the RtMidi Library.

The launch pad is treated as one array of buttons/LEDs, including scene launch and activator/solo/rec arm buttons. This is to make the usage of LEDs easier for use cases other than a MIDI controller.
They are accessed with x and y positions; the origin is located in the top left corner.
To access the pad buttons and LEDs use the "APC40_MAIN_PAD" type.

All other buttons, knobs and sliders have a corresponding definition.

# Examples

- RainDrops.cpp is a simple test application. It renders Rain Drops that wander down the main pad and split in two at the bottom.
- GTASAPC.cpp is an ASI Plugin for GTA SA that uses the APC as game controller (unfinished). Use the highlighted Pad Buttons to move, Cue Level and Device Control 5 to look around.

# Credits

APC40 documentation: https://www.akaipro.de/sites/default/files/2018-01/APC40_Communications_Protocol_rev_1.pdf_1db97c1fdba23bacf47df0f9bf64e913.pdf

RtMidi: https://www.music.mcgill.ca/~gary/rtmidi/

Dk22Pac - GTA Plugin SDK: https://github.com/DK22Pac/plugin-sdk/
