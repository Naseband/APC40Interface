#pragma once

#include  <RtMidi.h>

// ------------------------------------------------------------ Definitions

enum
{
	APC40_MAIN_PAD,

	APC40_BUTTON_TRACK_PAN,
	APC40_BUTTON_TRACK_SEND_A,
	APC40_BUTTON_TRACK_SEND_B,
	APC40_BUTTON_TRACK_SEND_C,

	APC40_BUTTON_SHIFT,

	APC40_BUTTON_BANK_UP,
	APC40_BUTTON_BANK_DOWN,
	APC40_BUTTON_BANK_LEFT,
	APC40_BUTTON_BANK_RIGHT,
	
	APC40_BUTTON_TAP_TEMPO,
	APC40_BUTTON_NUDGE_N,
	APC40_BUTTON_NUDGE_P,

	APC40_BUTTON_DEVICE_CLIP_TRACK,
	APC40_BUTTON_DEVICE_TOGGLE,
	APC40_BUTTON_DEVICE_LEFT,
	APC40_BUTTON_DEVICE_RIGHT,

	APC40_BUTTON_DETAIL_VIEW,
	APC40_BUTTON_REC_QUANTIZATION,
	APC40_BUTTON_MIDI_OVERDUB,
	APC40_BUTTON_METRONOME,

	APC40_BUTTON_PLAY,
	APC40_BUTTON_STOP,
	APC40_BUTTON_REC,

	APC40_SLIDER_CH1,
	APC40_SLIDER_CH2,
	APC40_SLIDER_CH3,
	APC40_SLIDER_CH4,
	APC40_SLIDER_CH5,
	APC40_SLIDER_CH6,
	APC40_SLIDER_CH7,
	APC40_SLIDER_CH8,
	APC40_SLIDER_CH_MASTER,
	APC40_SLIDER_CROSSFADE,

	APC40_KNOB_CUE_LEVEL,
	APC40_KNOB_TRACK1,
	APC40_KNOB_TRACK2,
	APC40_KNOB_TRACK3,
	APC40_KNOB_TRACK4,
	APC40_KNOB_TRACK5,
	APC40_KNOB_TRACK6,
	APC40_KNOB_TRACK7,
	APC40_KNOB_TRACK8,
	APC40_KNOB_DEVICE1,
	APC40_KNOB_DEVICE2,
	APC40_KNOB_DEVICE3,
	APC40_KNOB_DEVICE4,
	APC40_KNOB_DEVICE5,
	APC40_KNOB_DEVICE6,
	APC40_KNOB_DEVICE7,
	APC40_KNOB_DEVICE8
};

enum eAPC40LEDModes
{
	APC40_LED_MODE_OFF,
	APC40_LED_MODE_GREEN,
	APC40_LED_MODE_GREEN_BLINK,
	APC40_LED_MODE_RED,
	APC40_LED_MODE_RED_BLINK,
	APC40_LED_MODE_YELLOW,
	APC40_LED_MODE_YELLOW_BLINK,
	APC40_LED_MODE_ON = 127
};

// ------------------------------------------------------------ Structs

struct APC40Input
{
	int type;
	int x;
	int y;
	int value;
};

// ------------------------------------------------------------ 

template <typename T>
class APC40Interface 
{
private:
	RtMidiIn* m_pRtMidiIn = 0;
	RtMidiOut* m_pRtMidiOut = 0;

	bool m_bFoundIn = false;
	bool m_bFoundOut = false;

	unsigned int m_nPortIn = 0;
	unsigned int m_nPortOut = 0;

public:
	APC40Interface()
	{
		try
		{
			m_pRtMidiIn = new RtMidiIn();
			m_pRtMidiOut = new RtMidiOut();

			unsigned int nPorts = m_pRtMidiIn->getPortCount();

			for (unsigned i = 0; i < nPorts; i++)
			{
				std::string portName = m_pRtMidiIn->getPortName(i);

				if (!m_bFoundIn && portName.find("Akai APC40") == 0)
				{
					m_nPortIn = i;
					m_bFoundIn = true;

					std::cout << "Found APC 40 Input (Port " << i << ")\n";
				}
			}

			if (m_bFoundIn)
			{
				m_pRtMidiIn->openPort(m_nPortIn);
			}

			// ---

			nPorts = m_pRtMidiOut->getPortCount();

			for (unsigned i = 0; i < nPorts; i++)
			{
				std::string portName = m_pRtMidiOut->getPortName(i);

				if (!m_bFoundOut && portName.find("Akai APC40") == 0)
				{
					m_nPortOut = i;
					m_bFoundOut = true;

					std::cout << "Found APC 40 Output (Port " << i << ")\n";
				}
			}

			if (m_bFoundOut)
			{
				m_pRtMidiOut->openPort(m_nPortOut);
			}
		}
		catch (RtMidiError& error)
		{
			error.printMessage();
		}
	}

	~APC40Interface()
	{
		delete m_pRtMidiIn;
		delete m_pRtMidiOut;
	}

	void InitDevice(unsigned char mode = 0x42)
	{
		std::vector<unsigned char> message;

		message.push_back(0xF0); // MIDI excl start
		message.push_back(0x47); // Manu ID
		message.push_back(0x7F); // DevID
		message.push_back(0x73); // Prod Model ID
		message.push_back(0x60); // Msg Type ID (0x60=Init?)
		message.push_back(0x00); // Num Data Bytes (most sign.)
		message.push_back(0x04); // Num Data Bytes (least sign.)
		message.push_back(mode); // Device Mode (0x40=unset, 0x41=Ableton, 0x42=Ableton with full ctrl)
		message.push_back(0x01); // PC Ver Major (?)
		message.push_back(0x01); // PC Ver Minor (?)
		message.push_back(0x01); // PC Bug Fix Lvl (?)
		message.push_back(0xF7); // MIDI excl end

		m_pRtMidiOut->sendMessage(&message);
	}

	void ResetLEDs()
	{
		for (int x = 0; x < 9; x++)
		{
			for (int y = 0; y < 10; y++)
			{
				this->SetLEDMode(APC40_MAIN_PAD, x, y, APC40_LED_MODE_OFF);
			}
		}
	}

	void SetCallbackFunc(RtMidiIn::RtMidiCallback callback_func)
	{
		m_pRtMidiIn->setCallback(callback_func);
		m_pRtMidiIn->ignoreTypes(false, false, false);
	}

	// ------- Input

	APC40Input GetInputFromMIDIMessage(double deltatime, std::vector< unsigned char >* message)
	{
		unsigned int nBytes = message->size();

		int type = -1, x = 0, y = 0, value = 0;

		if (nBytes == 3)
		{
			int b1 = (int)message->at(0);
			int b2 = (int)message->at(1);
			int b3 = (int)message->at(2); 

			if (b1 >= 0x90 && b1 <= 0x97 && b2 >= 0x34 && b2 <= 0x39) // Clips
			{
				type = APC40_MAIN_PAD;
				x = b1 - 0x90;
				y = b2 - 0x35;

				if (y == -1)
					y = 5;
			}
			else if (b1 == 0x90 && b2 >= 0x51 && b2 <= 0x56) // Scene Launch + Stop All Clips
			{
				type = APC40_MAIN_PAD;

				x = 8;
				y = b2 - 0x52;

				if (y == -1)
					y = 5;
			}
			else if (b1 >= 0x90 && b1 <= 0x97 && b2 == 0x33) // Track Selection
			{
				type = APC40_MAIN_PAD;

				x = b1 - 0x90;
				y = 6;
			}
			else if (b1 == 0x90 && b2 == 0x50) // Master
			{
				type = APC40_MAIN_PAD;

				x = 8;
				y = 6;
			}
			else if (b1 >= 0x90 && b1 <= 0x97 && b2 >= 0x30 && b2 <= 0x32) // Activator, Solo, Rec Arm
			{
				type = APC40_MAIN_PAD;

				x = b1 - 0x90;
				y = 9 - (b2 - 0x30);
			}
			else if (b1 == 0xB0 && b2 == 0x2F) // Cue Level
			{
				type = APC40_KNOB_CUE_LEVEL;

				value = b3;
			}
			else if (b1 == 0x90 && b2 == 0x62) // Shift
			{
				type = APC40_BUTTON_SHIFT; 
			}
		}

		APC40Input input;

		input.type = type;
		input.x = x;
		input.y = y;
		input.value = value;

		return input;
	}

	// ------- Output

	bool SetLEDMode(int type, int x, int y, int value)
	{
		unsigned char b1 = 0;
		unsigned char b2 = 0;

		switch (type)
		{
		case APC40_MAIN_PAD:
			if (x < 0 || x > 8 || y < 0 || y > 9)
				return false;

			if (x >= 0 && x <= 7 && y >= 0 && y <= 4) // Clip Launch
			{
				b1 = 0x90 + x;
				b2 = 0x35 + y;
			}
			else if (x == 8 && y >= 0 && y <= 4) // Scene Launch
			{
				b1 = 0x90;
				b2 = 0x52 + y;
			}
			else if (x >= 0 && x <= 7 && y == 5) // Clip Stop
			{
				b1 = 0x90 + x;
				b2 = 0x34;
			}
			else if (x == 8 && y == 5) // Stop All Clips (? missing in docs, 0x51 is used by Ableton but does nothing in mode 2)
			{
				b1 = 0x90;
				b2 = 0x51;
			}
			else if (x >= 0 && x <= 7 && y == 6) // Track Selection
			{
				b1 = 0x90 + x;
				b2 = 0x33;
			}
			else if (x == 8 && y == 6) // Master
			{
				b1 = 0x90;
				b2 = 0x50;
			}
			else if (x >= 0 && x <= 7 && y == 7) // Activator
			{
				b1 = 0x90 + x;
				b2 = 0x32;
			}
			else if (x >= 0 && x <= 7 && y == 8) // Solo
			{
				b1 = 0x90 + x;
				b2 = 0x31;
			}
			else if (x >= 0 && x <= 7 && y == 9) // Rec Arm
			{
				b1 = 0x90 + x;
				b2 = 0x30;
			}
			break;


		case APC40_BUTTON_TRACK_PAN:
			break;

		case APC40_BUTTON_TRACK_SEND_A:
			break;

		case APC40_BUTTON_TRACK_SEND_B:
			break;

		case APC40_BUTTON_TRACK_SEND_C:
			break;


		case APC40_BUTTON_DEVICE_CLIP_TRACK:
			break;

		case APC40_BUTTON_DEVICE_TOGGLE:
			break;

		case APC40_BUTTON_DEVICE_LEFT:
			break;

		case APC40_BUTTON_DEVICE_RIGHT:
			break;


		case APC40_BUTTON_DETAIL_VIEW:
			break;

		case APC40_BUTTON_REC_QUANTIZATION:
			break;

		case APC40_BUTTON_MIDI_OVERDUB:
			break;

		case APC40_BUTTON_METRONOME:
			break;
		}

		if (b1 == 0)
			return false;

		std::vector<unsigned char> message;

		message.push_back(b1);
		message.push_back(b2);
		message.push_back(static_cast<unsigned char>(value));

		m_pRtMidiOut->sendMessage(&message);

		return true;
	}

	bool SetLEDMode(int type, int value)
	{
		return SetLEDMode(type, 0, 0, value);
	}
};
