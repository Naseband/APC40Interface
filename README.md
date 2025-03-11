# APC40Interface

C++ Library to interact with the Akai APC40 (portable MIDI controller).

This library is only compatible with the very old APC40 from 2009, not the Akai APC40 mini (or mini MkII).

Since version 2, this library is no longer responsible for sending midi messages.

It merely keeps track of the APC40's state and generates a midi buffer for you to send to the device, aswell as convert raw midi input you receive from your midi library to APC40 relevant messages.
Output messages are generated only if the state of a control changed from the previous state.

All device buttons, LEDs, knobs, etc. are uniquely identified by a control identifier.

In addition, some controls are grouped together and have an extra id or coordinate.
These ids/coordnates are packed into the eAPC40Control value (meaning they are offset from the base value).

These are:

- Volume Sliders (id 0-8, where 8 is the Master slider)
- Track Knobs (id 0-7)
- Device Knobs (id 0-7)
- Pad (X coordinates/columns 0-8 and Y coordinates/rows 0-9)

Note that column 8 of the pad (the scene launch column) has only 6 valid LEDs, the MASTER button LED cannot be set (its input does still work).

There are helper functions to pack/unpack or strip the ID/coordinates of eAPC40Control identifiers.

To set the value of track knob 1, you can pack the id into the eAPC40Control identifier:

```cpp
eAPC40Control packed_control = APC40PackControl(eAPC40Control::TrackKnobValue, 1); // Knob 1 = second knob (they start at 0)

APC40Interface apc40;
apc40.SetControlValue(packed_control, 127); // Update the state
```

Note that APC40PackControl, its overloads and other utility functions are constexpr functions outside of the APC40Interface class.
So in the example above packed_control could actually be constexpr, or passed directly to SetControlValue to avoid extra runtime cost.


To generate output messages, you can call GetMidiMessages. From there you need to use a library such as RtMidi to send them to the actual device:

```cpp
std::vector<unsigned char> midi_messages;
apc40.GetMidiMessages(midi_messages, true, true);

// Send the message buffer if not empty.
```


When you receive midi input from the device, the messages are always 3 bytes long. You can easily translate them from raw midi messages to an APC40Input message:

```cpp
void YourMidiMessageInputCallback(unsigned char* midi_message, size_t message_size)
{
	APC40Input input;

	// If the function returns false it means that the input message is not supported and can be discarded

	if(apc40.TranslateInputMessage(midi_message, message_size, input))
	{
		if(input.control == eAPC40Control::Play) // Play button was pressed
		{
			// ...
		}
	}
}
```


To handle messages from for example the launch pad, you need to strip the coordinates from the control identifier and unpack the X/Y coordnates:

```cpp
APC40Input input; // APC40Input is a struct

if(apc40.TranslateInputMessage(midi_message, message_size, input))
{
	if(APC40StripControl(input.control) == eAPC40Control::Pad) // A pad button was pressed
	{
		int x = APC40UnpackControlX(input.control);
		int y = APC40UnpackControlY(input.control);

		// Do something with x and y!
	}
}
```

Before all functions and LEDs of the APC40 can be accessed, it has to be reset and put into a special mode (called Ableton Full Control in the documentation).

This can easily be done by sending an initialization message to the APC40. This message can be obtained from the interface:

```cpp
std::vector<unsigned char> midi_message;
apc40.GetInitMessage(midi_message);

// Send the message to the device. It needs to be sent as SysEx/LongMessage.
```
This has to be done everytime the APC40 has been disconnected or after it was initialized in a different mode (ie. Ableton).

Once this message is sent, the APC40 will be in a clean state, as in all LEDs are off and knob modes are reset to default.
The APC40 will also send a bunch of input messages, which are related to the current slider positions.

If you want to utilize these initial slider values, you should make sure to have your input callback/queue connected before sending the initialization message.

If the APC40 is disconnected and reconnected you will also have to clear the current state of the interface, so that the desired state can be synced correctly with the APC40. See APC40Interface::ResetCurrentState().

# Midi libraries

There are various midi libraries, they should all work well with the interface. You do however need a library that supports SysEx messages.

On Windows, I would strongly recommend either using the winrt midi functions or use WinMM directly (SendLongMessage for all types of messages).

Most libraries (such as libremidi, RtMidi, etc) use WinMM's SendShortMsg as a backend on Windows which is heavily throttled on Win10 and Win11 unless you are really far behind on updates.

# Examples

- RainDrops.cpp is a simple test application. It renders Rain Drops that wander down the main pad and split in two at the bottom [CURRENTLY OUTDATED].

# References

[APC40 Communcation Protocol](https://cdn.inmusicbrands.com/akai/apc40/APC40_Communications_Protocol_rev_1.pdf_1db97c1fdba23bacf47df0f9bf64e913.pdf)
