# APC40Interface

C++ Library to interact with the Akai APC 40 (portable MIDI controller).

Uses the RtMidi Library.

The launch pad is treated as one array of buttons/LEDs, including scene launch and activator/solo/rec arm buttons. This is to make the usage of LEDs easier for use cases other than MIDI controller.
They are accessed with x and y positions; the origin is located in the top left corner.
To access the pad buttons and LEDs use the "APC40_MAIN_PAD" type.

All other buttons, knobs and sliders have a corresponding definition.

# Examples

- Test.cpp is a simple test application. It shows some debug info if enabled and tests all LEDs, knobs and button inputs/outputs.
- GTASAPC.cpp is an ASI Plugin for GTA SA that uses the APC as game controller (unfinished).

# Credits

APC40 documentation: https://www.akaipro.de/sites/default/files/2018-01/APC40_Communications_Protocol_rev_1.pdf_1db97c1fdba23bacf47df0f9bf64e913.pdf
RtMidi: https://www.music.mcgill.ca/~gary/rtmidi/
