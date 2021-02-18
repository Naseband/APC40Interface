#pragma once

/*

APC40 Interface Library

*/

#include  <RtMidi.h>

// ------------------------------------------------------------ Definitions

enum
{
	APC40_BUTTON_PAD, // All rubber buttons, including scene launch, solo, etc. (identified by x,y coordinates)

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
	APC40_BUTTON_DEVICE_DETAIL_VIEW,
	APC40_BUTTON_DEVICE_REC_QUANTIZATION,
	APC40_BUTTON_DEVICE_MIDI_OVERDUB,
	APC40_BUTTON_DEVICE_METRONOME,

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
	APC40_SLIDER_MASTER,
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

enum 
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

enum 
{
	APC40_KNOB_MODE_OFF,
	APC40_KNOB_MODE_SINGLE,
	APC40_KNOB_MODE_VOLUME,
	APC40_KNOB_MODE_PAN
};

enum
{
	APC40_IO_ALL = -1,
	APC40_IO_IN = 0,
	APC40_IO_OUT = 1
};


#define APC40_INPUT_TYPE_INVALID		-1 // For GetAPC40InputFromMidiMessage(Queue), invalid signal
#define APC40_INPUT_TYPE_EMPTY			-2 // For GetAPC40InputFromMidiMessageQueue, queue is empty

// ------------------------------------------------------------ Structs and Enums

struct APC40Input
{
	int type = APC40_INPUT_TYPE_INVALID;
	unsigned char x = 0;
	unsigned char y = 0;
	unsigned char value = 0;
	bool pressed = false;
};

enum class APC40ErrorCode : unsigned int
{
	ERRCODE_MIDI_IN, ERRCODE_MIDI_OUT
};

struct APC40Error
{
	APC40ErrorCode eErrCode;
	char* szText = NULL;
};

// ------------------------------------------------------------ 

class APC40Interface 
{
private:
	RtMidiIn* m_pRtMidiIn = NULL;
	RtMidiOut* m_pRtMidiOut = NULL;

	bool m_bFoundIn = false;
	bool m_bFoundOut = false;

	unsigned int m_nPortIn = 0;
	unsigned int m_nPortOut = 0;

	unsigned char m_nLEDStates[8][10] = { APC40_LED_MODE_OFF };

	std::vector<APC40Error> m_vecErrorList;

public:
	APC40Interface()
	{
		try
		{
			m_pRtMidiIn = new RtMidiIn();
			m_bFoundIn = false;

			unsigned int nPorts = m_pRtMidiIn->getPortCount();

			for (unsigned i = 0; i < nPorts; i++)
			{
				std::string portName = m_pRtMidiIn->getPortName(i);

				if (!m_bFoundIn && portName.find("Akai APC40") == 0)
				{
					m_nPortIn = i;
					m_bFoundIn = true;
				}
			}

			if (m_bFoundIn)
			{
				m_pRtMidiIn->openPort(m_nPortIn);
			}
		}
		catch (RtMidiError& error)
		{
			auto errtext = error.getMessage();
			auto len = errtext.length();
			auto tmp = new char[len + 1];
			errtext.copy(tmp, len);
			tmp[len] = 0;

			APC40Error new_error;

			new_error.eErrCode = APC40ErrorCode::ERRCODE_MIDI_IN;
			new_error.szText = tmp;

			m_vecErrorList.push_back(new_error);
		}

		try
		{
			m_pRtMidiOut = new RtMidiOut();
			m_bFoundOut = false;

			unsigned int nPorts = m_pRtMidiOut->getPortCount();

			for (unsigned i = 0; i < nPorts; i++)
			{
				std::string portName = m_pRtMidiOut->getPortName(i);

				if (!m_bFoundOut && portName.find("Akai APC40") == 0)
				{
					m_nPortOut = i;
					m_bFoundOut = true;
				}
			}

			if (m_bFoundOut)
			{
				m_pRtMidiOut->openPort(m_nPortOut);
			}
		}
		catch (RtMidiError& error)
		{
			auto errtext = error.getMessage();
			auto len = errtext.length();
			auto tmp = new char[len + 1];
			errtext.copy(tmp, len);
			tmp[len] = 0;

			APC40Error new_error;

			new_error.eErrCode = APC40ErrorCode::ERRCODE_MIDI_OUT;
			new_error.szText = tmp;

			m_vecErrorList.push_back(new_error);
		}
	}

	~APC40Interface()
	{
		for (size_t i = 0U, s = m_vecErrorList.size(); i < s; ++i)
		{
			delete[] m_vecErrorList[i].szText;
		}

		m_vecErrorList.clear();

		if(m_pRtMidiIn != NULL)
			delete m_pRtMidiIn;

		if(m_pRtMidiOut != NULL)
			delete m_pRtMidiOut;
	}

	void InitDevice(unsigned char mode = 0x42)
	{
		if (!m_bFoundOut)
			return;

		std::vector<unsigned char> message;

		message.push_back(0xF0); // MIDI excl start
		message.push_back(0x47); // Manu ID
		message.push_back(0x7F); // DevID
		message.push_back(0x73); // Prod Model ID
		message.push_back(0x60); // Msg Type ID (0x60=Init)
		message.push_back(0x00); // Num Data Bytes (most sign.)
		message.push_back(0x04); // Num Data Bytes (least sign.)
		message.push_back(mode); // Device Mode (0x40=unset, 0x41=Ableton, 0x42=Ableton with full ctrl)
		message.push_back(0x01); // PC Ver Major (?)
		message.push_back(0x01); // PC Ver Minor (?)
		message.push_back(0x01); // PC Bug Fix Lvl (?)
		message.push_back(0xF7); // MIDI excl end

		m_pRtMidiOut->sendMessage(&message);
	}

	void ResetLEDs(bool main_pad = true, bool track = true, bool device = true)
	{
		if (!m_bFoundOut)
			return;

		if (main_pad)
		{
			for (int x = 0; x < 9; x++)
			{
				for (int y = 0; y < 10; y++)
				{
					this->SetLEDMode(APC40_BUTTON_PAD, x, y, APC40_LED_MODE_OFF);
				}
			}
		}

		if (track)
		{
			this->SetLEDMode(APC40_BUTTON_TRACK_PAN, APC40_LED_MODE_OFF);
			this->SetLEDMode(APC40_BUTTON_TRACK_SEND_A, APC40_LED_MODE_OFF);
			this->SetLEDMode(APC40_BUTTON_TRACK_SEND_B, APC40_LED_MODE_OFF);
			this->SetLEDMode(APC40_BUTTON_TRACK_SEND_C, APC40_LED_MODE_OFF);
		}

		if (device)
		{
			this->SetLEDMode(APC40_BUTTON_DEVICE_CLIP_TRACK, APC40_LED_MODE_OFF);
			this->SetLEDMode(APC40_BUTTON_DEVICE_TOGGLE, APC40_LED_MODE_OFF);
			this->SetLEDMode(APC40_BUTTON_DEVICE_LEFT, APC40_LED_MODE_OFF);
			this->SetLEDMode(APC40_BUTTON_DEVICE_RIGHT, APC40_LED_MODE_OFF);
			this->SetLEDMode(APC40_BUTTON_DEVICE_DETAIL_VIEW, APC40_LED_MODE_OFF);
			this->SetLEDMode(APC40_BUTTON_DEVICE_REC_QUANTIZATION, APC40_LED_MODE_OFF);
			this->SetLEDMode(APC40_BUTTON_DEVICE_MIDI_OVERDUB, APC40_LED_MODE_OFF);
			this->SetLEDMode(APC40_BUTTON_DEVICE_METRONOME, APC40_LED_MODE_OFF);
		}
	}

	void ResetKnobs(bool track = true, bool device = true)
	{
		if (!m_bFoundOut)
			return;

		if (track)
		{
			this->SetKnobMode(APC40_KNOB_TRACK1, APC40_KNOB_MODE_OFF);
			this->SetKnobMode(APC40_KNOB_TRACK2, APC40_KNOB_MODE_OFF);
			this->SetKnobMode(APC40_KNOB_TRACK3, APC40_KNOB_MODE_OFF);
			this->SetKnobMode(APC40_KNOB_TRACK4, APC40_KNOB_MODE_OFF);
			this->SetKnobMode(APC40_KNOB_TRACK5, APC40_KNOB_MODE_OFF);
			this->SetKnobMode(APC40_KNOB_TRACK6, APC40_KNOB_MODE_OFF);
			this->SetKnobMode(APC40_KNOB_TRACK7, APC40_KNOB_MODE_OFF);
			this->SetKnobMode(APC40_KNOB_TRACK8, APC40_KNOB_MODE_OFF);
		}

		if (device)
		{
			this->SetKnobMode(APC40_KNOB_DEVICE1, APC40_KNOB_MODE_OFF);
			this->SetKnobMode(APC40_KNOB_DEVICE2, APC40_KNOB_MODE_OFF);
			this->SetKnobMode(APC40_KNOB_DEVICE3, APC40_KNOB_MODE_OFF);
			this->SetKnobMode(APC40_KNOB_DEVICE4, APC40_KNOB_MODE_OFF);
			this->SetKnobMode(APC40_KNOB_DEVICE5, APC40_KNOB_MODE_OFF);
			this->SetKnobMode(APC40_KNOB_DEVICE6, APC40_KNOB_MODE_OFF);
			this->SetKnobMode(APC40_KNOB_DEVICE7, APC40_KNOB_MODE_OFF);
			this->SetKnobMode(APC40_KNOB_DEVICE8, APC40_KNOB_MODE_OFF);
		}
	}

	void SetCallbackFunc(RtMidiIn::RtMidiCallback callback_func)
	{
		m_pRtMidiIn->setCallback(callback_func);
		m_pRtMidiIn->ignoreTypes(false, false, false);
	}

	void CancelCallbackFunc()
	{
		m_pRtMidiIn->cancelCallback();
	}

	bool IsConnected(int io_type = APC40_IO_ALL)
	{
		if (io_type == APC40_IO_ALL && this->m_bFoundIn && this->m_bFoundOut)
			return true;
		else if (io_type == APC40_IO_IN && this->m_bFoundIn)
			return true;
		else if (io_type == APC40_IO_OUT && this->m_bFoundOut)
			return true;

		return false;
	}

	// ------- Input

	APC40Input GetAPC40InputFromMidiMessageQueue()
	{
		std::vector <unsigned char> message;

		double deltatime = m_pRtMidiIn->getMessage(&message);

		return GetAPC40InputFromMIDIMessage(deltatime, &message);
	}

	APC40Input GetAPC40InputFromMIDIMessage(double deltatime, std::vector<unsigned char>* message)
	{
		unsigned int nBytes = message->size();

		int type = -1, x = 0, y = 0, value = 0;
		bool pressed = true;

		if (nBytes == 3)
		{
			int b1 = (int)message->at(0);
			int b2 = (int)message->at(1);
			int b3 = (int)message->at(2);

			if (b1 >= 0x80 && b1 <= 0x8F)
			{
				b1 += 0x10;
				pressed = false;
			}

			// Main Pad

			if (b1 >= 0x90 && b1 <= 0x97 && b2 >= 0x34 && b2 <= 0x39) // Clips
			{
				type = APC40_BUTTON_PAD;
				x = b1 - 0x90;
				y = b2 - 0x35;

				if (y == -1)
					y = 5;
			}
			else if (b1 == 0x90 && b2 >= 0x51 && b2 <= 0x56) // Scene Launch + Stop All Clips
			{
				type = APC40_BUTTON_PAD;

				x = 8;
				y = b2 - 0x52;

				if (y == -1)
					y = 5;
			}
			else if (b1 >= 0x90 && b1 <= 0x97 && b2 == 0x33) // Track Selection
			{
				type = APC40_BUTTON_PAD;

				x = b1 - 0x90;
				y = 6;
			}
			else if (b1 == 0x90 && b2 == 0x50) // Master
			{
				type = APC40_BUTTON_PAD;

				x = 8;
				y = 6;
			}
			else if (b1 >= 0x90 && b1 <= 0x97 && b2 >= 0x30 && b2 <= 0x32) // Activator, Solo, Rec Arm
			{
				type = APC40_BUTTON_PAD;

				x = b1 - 0x90;
				y = 9 - (b2 - 0x30);
			}
			else if (b1 == 0x90 && b2 == 0x57) // Track Control
			{
				type = APC40_BUTTON_TRACK_PAN;
			}
			else if (b1 == 0x90 && b2 == 0x58)
			{
				type = APC40_BUTTON_TRACK_SEND_A;
			}
			else if (b1 == 0x90 && b2 == 0x59)
			{
				type = APC40_BUTTON_TRACK_SEND_B;
			}
			else if (b1 == 0x90 && b2 == 0x5A)
			{
				type = APC40_BUTTON_TRACK_SEND_C;
			}
			else if (b1 == 0xB0 && b2 == 0x30)
			{
				type = APC40_KNOB_TRACK1;
				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x31)
			{
				type = APC40_KNOB_TRACK2;
				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x32)
			{
				type = APC40_KNOB_TRACK3;
				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x33)
			{
				type = APC40_KNOB_TRACK4;
				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x34)
			{
				type = APC40_KNOB_TRACK5;
				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x35)
			{
				type = APC40_KNOB_TRACK6;
				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x36)
			{
				type = APC40_KNOB_TRACK7;
				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x37)
			{
				type = APC40_KNOB_TRACK8;
				value = b3;
			}
			else if (b1 == 0x90 && b2 == 0x3A) // Device Control
			{
				type = APC40_BUTTON_DEVICE_CLIP_TRACK;
			}
			else if (b1 == 0x90 && b2 == 0x3B)
			{
				type = APC40_BUTTON_DEVICE_TOGGLE;
			}
			else if (b1 == 0x90 && b2 == 0x3C)
			{
				type = APC40_BUTTON_DEVICE_LEFT;
			}
			else if (b1 == 0x90 && b2 == 0x3D)
			{
				type = APC40_BUTTON_DEVICE_RIGHT;
			}
			else if (b1 == 0x90 && b2 == 0x3E)
			{
			type = APC40_BUTTON_DEVICE_DETAIL_VIEW;
			}
			else if (b1 == 0x90 && b2 == 0x3F)
			{
			type = APC40_BUTTON_DEVICE_REC_QUANTIZATION;
			}
			else if (b1 == 0x90 && b2 == 0x40)
			{
			type = APC40_BUTTON_DEVICE_MIDI_OVERDUB;
			}
			else if (b1 == 0x90 && b2 == 0x41)
			{
			type = APC40_BUTTON_DEVICE_METRONOME;
			}
			else if (b1 == 0xB0 && b2 == 0x10)
			{
				type = APC40_KNOB_DEVICE1;
				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x11)
			{
				type = APC40_KNOB_DEVICE2;
				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x12)
			{
				type = APC40_KNOB_DEVICE3;
				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x13)
			{
				type = APC40_KNOB_DEVICE4;
				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x14)
			{
				type = APC40_KNOB_DEVICE5;
				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x15)
			{
				type = APC40_KNOB_DEVICE6;
				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x16)
			{
				type = APC40_KNOB_DEVICE7;
				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x17)
			{
				type = APC40_KNOB_DEVICE8;
				value = b3;
			}
			else if (b1 == 0x90 && b2 == 0x62) // Misc
			{
				type = APC40_BUTTON_SHIFT;
			}
			else if (b1 == 0x90 && b2 == 0x61)
			{
				type = APC40_BUTTON_BANK_LEFT;
			}
			else if (b1 == 0x90 && b2 == 0x60)
			{
				type = APC40_BUTTON_BANK_RIGHT;
			}
			else if (b1 == 0x90 && b2 == 0x5E)
			{
				type = APC40_BUTTON_BANK_UP;
			}
			else if (b1 == 0x90 && b2 == 0x5F)
			{
				type = APC40_BUTTON_BANK_DOWN;
			}
			else if (b1 == 0x90 && b2 == 0x63)
			{
				type = APC40_BUTTON_TAP_TEMPO;
			}
			else if (b1 == 0x90 && b2 == 0x65)
			{
				type = APC40_BUTTON_NUDGE_N;
			}
			else if (b1 == 0x90 && b2 == 0x64)
			{
				type = APC40_BUTTON_NUDGE_P;
			}
			else if (b1 == 0x90 && b2 == 0x5B)
			{
				type = APC40_BUTTON_PLAY;
			}
			else if (b1 == 0x90 && b2 == 0x5C)
			{
				type = APC40_BUTTON_STOP;
			}
			else if (b1 == 0x90 && b2 == 0x5D)
			{
				type = APC40_BUTTON_REC;
			}
			else if (b1 == 0xB0 && b2 == 0x2F)
			{
				type = APC40_KNOB_CUE_LEVEL;

				value = b3;
			}
			else if (b1 == 0xB0 && b2 == 0x07)
			{
				type = APC40_SLIDER_CH1;
			}
			else if (b1 == 0xB1 && b2 == 0x07)
			{
				type = APC40_SLIDER_CH2;
			}
			else if (b1 == 0xB2 && b2 == 0x07)
			{
				type = APC40_SLIDER_CH3;
			}
			else if (b1 == 0xB3 && b2 == 0x07)
			{
				type = APC40_SLIDER_CH4;
			}
			else if (b1 == 0xB4 && b2 == 0x07)
			{
				type = APC40_SLIDER_CH5;
			}
			else if (b1 == 0xB5 && b2 == 0x07)
			{
				type = APC40_SLIDER_CH6;
			}
			else if (b1 == 0xB6 && b2 == 0x07)
			{
				type = APC40_SLIDER_CH7;
			}
			else if (b1 == 0xB7 && b2 == 0x07)
			{
				type = APC40_SLIDER_CH8;
			}
			else if (b1 == 0xB0 && b2 == 0x0E)
			{
				type = APC40_SLIDER_MASTER;
			}
			else if (b1 == 0xB0 && b2 == 0x0F)
			{
				type = APC40_SLIDER_CROSSFADE;
			}
		}
		else if (nBytes == 0)
		{
			type = APC40_INPUT_TYPE_EMPTY;
		}

		APC40Input input;

		input.type = type;
		input.x = (unsigned char)x;
		input.y = (unsigned char)y;
		input.value = (unsigned char)value;
		input.pressed = pressed;

		return input;
	}

	// ------- Output

	bool SetLEDMode(int type, int x, int y, int value)
	{
		unsigned char b1 = 0;
		unsigned char b2 = 0;

		switch (type)
		{
		case APC40_BUTTON_PAD:
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
			m_nLEDStates[x][y] = (unsigned char)value;
			break;


		case APC40_BUTTON_TRACK_PAN:
			b1 = 0x90;
			b2 = 0x57;
			break;

		case APC40_BUTTON_TRACK_SEND_A:
			b1 = 0x90;
			b2 = 0x58;
			break;

		case APC40_BUTTON_TRACK_SEND_B:
			b1 = 0x90;
			b2 = 0x59;
			break;

		case APC40_BUTTON_TRACK_SEND_C:
			b1 = 0x90;
			b2 = 0x5A;
			break;


		case APC40_BUTTON_DEVICE_CLIP_TRACK:
			b1 = 0x90;
			b2 = 0x3A;
			break;

		case APC40_BUTTON_DEVICE_TOGGLE:
			b1 = 0x90;
			b2 = 0x3B;
			break;

		case APC40_BUTTON_DEVICE_LEFT:
			b1 = 0x90;
			b2 = 0x3C;
			break;

		case APC40_BUTTON_DEVICE_RIGHT:
			b1 = 0x90;
			b2 = 0x3D;
			break;

		case APC40_BUTTON_DEVICE_DETAIL_VIEW:
			b1 = 0x90;
			b2 = 0x3E;
			break;

		case APC40_BUTTON_DEVICE_REC_QUANTIZATION:
			b1 = 0x90;
			b2 = 0x3F;
			break;

		case APC40_BUTTON_DEVICE_MIDI_OVERDUB:
			b1 = 0x90;
			b2 = 0x40;
			break;

		case APC40_BUTTON_DEVICE_METRONOME:
			b1 = 0x90;
			b2 = 0x41;
			break;
		}

		if (!m_bFoundOut || b1 == 0)
			return false;

		std::vector<unsigned char> message;

		if (b1 >= 0x90 && b1 < 0xA0 && value == 0)
		{
			message.push_back(b1 - 0x10);
			message.push_back(b2);
			message.push_back(0);
		}
		else
		{
			message.push_back(b1);
			message.push_back(b2);
			message.push_back((unsigned int)value);
		}

		m_pRtMidiOut->sendMessage(&message);

		return true;
	}

	bool SetLEDMode(int type, int value)
	{
		return SetLEDMode(type, 0, 0, value);
	}

	void GetLEDStates(unsigned char (&led_states)[8][10])
	{
		for (int x = 0; x < 8; ++x)
			for (int y = 0; y < 10; ++y)
				led_states[x][y] = m_nLEDStates[x][y];
	}

	bool GetLEDCircularPos(int pos, int *x, int *y)
	{
		if (pos < 0 || pos >= 32)
			return false;

		if (pos >= 0 && pos <= 7) // Horizontal Top
		{
			*x = pos;
			*y = 0;
		}
		else if (pos >= 16 && pos <= 23) // Horizontal Bottom
		{
			*x = 7 - (pos - 16);
			*y = 9;
		}
		else if (pos >= 7 && pos <= 16) // Vertical Right
		{
			*x = 7;
			*y = pos - 7;
		}
		else if (pos >= 23 && pos <= 31) // Vertical Left
		{
			*x = 0;
			*y = 9 - (pos - 23);
		}

		return true;
	}

	bool IsValidLEDPos(int x, int y, bool allow_scene = true)
	{
		if (allow_scene && x == 8 && y >= 0 && y < 7)
			return true;

		if (x < 0 || x > 7 || y < 0 || y > 9)
			return false;

		return true;
	}

	bool SetKnobMode(int type, int value)
	{
		if (!m_bFoundOut)
			return false;

		unsigned char b2 = 0;

		switch (type)
		{
		case APC40_KNOB_TRACK1:
			b2 = 0x38;
			break;
		case APC40_KNOB_TRACK2:
			b2 = 0x39;
			break;
		case APC40_KNOB_TRACK3:
			b2 = 0x3A;
			break;
		case APC40_KNOB_TRACK4:
			b2 = 0x3B;
			break;
		case APC40_KNOB_TRACK5:
			b2 = 0x3C;
			break;
		case APC40_KNOB_TRACK6:
			b2 = 0x3D;
			break;
		case APC40_KNOB_TRACK7:
			b2 = 0x3E;
			break;
		case APC40_KNOB_TRACK8:
			b2 = 0x3F;
			break;
		case APC40_KNOB_DEVICE1:
			b2 = 0x18;
			break;
		case APC40_KNOB_DEVICE2:
			b2 = 0x19;
			break;
		case APC40_KNOB_DEVICE3:
			b2 = 0x1A;
			break;
		case APC40_KNOB_DEVICE4:
			b2 = 0x1B;
			break;
		case APC40_KNOB_DEVICE5:
			b2 = 0x1C;
			break;
		case APC40_KNOB_DEVICE6:
			b2 = 0x1D;
			break;
		case APC40_KNOB_DEVICE7:
			b2 = 0x1E;
			break;
		case APC40_KNOB_DEVICE8:
			b2 = 0x1F;
			break;
		}

		if (b2 == 0)
			return false;

		std::vector<unsigned char> message;

		message.push_back(0xB0);
		message.push_back(b2);
		message.push_back((unsigned char)value);

		m_pRtMidiOut->sendMessage(&message);

		return true;
	}

	bool SetKnobValue(int type, int value)
	{
		if (!m_bFoundOut)
			return false;

		unsigned char b2 = 0;

		switch (type)
		{
		case APC40_KNOB_TRACK1:
			b2 = 0x30;
			break;
		case APC40_KNOB_TRACK2:
			b2 = 0x31;
			break;
		case APC40_KNOB_TRACK3:
			b2 = 0x32;
			break;
		case APC40_KNOB_TRACK4:
			b2 = 0x33;
			break;
		case APC40_KNOB_TRACK5:
			b2 = 0x34;
			break;
		case APC40_KNOB_TRACK6:
			b2 = 0x35;
			break;
		case APC40_KNOB_TRACK7:
			b2 = 0x36;
			break;
		case APC40_KNOB_TRACK8:
			b2 = 0x37;
			break;
		case APC40_KNOB_DEVICE1:
			b2 = 0x10;
			break;
		case APC40_KNOB_DEVICE2:
			b2 = 0x11;
			break;
		case APC40_KNOB_DEVICE3:
			b2 = 0x12;
			break;
		case APC40_KNOB_DEVICE4:
			b2 = 0x13;
			break;
		case APC40_KNOB_DEVICE5:
			b2 = 0x14;
			break;
		case APC40_KNOB_DEVICE6:
			b2 = 0x15;
			break;
		case APC40_KNOB_DEVICE7:
			b2 = 0x16;
			break;
		case APC40_KNOB_DEVICE8:
			b2 = 0x17;
			break;
		}

		if (b2 == 0)
			return false;

		std::vector<unsigned char> message;

		message.push_back(0xB0);
		message.push_back(b2);
		message.push_back((unsigned char)value);

		m_pRtMidiOut->sendMessage(&message);

		return true;
	}

	int GetKnobValueScaled(int knob_value, int min, int max)
	{
		return min + knob_value * (max - min) / 127;
	}

	float GetKnobValueScaled(int knob_value, float min, float max)
	{
		return min + ((float)knob_value * (max - min) / 127.0f);
	}

	int GetKnobValueLEDCount(int knob_value, int knob_mode)
	{
		switch (knob_mode)
		{
		case APC40_KNOB_MODE_SINGLE:
			return 1;

		case APC40_KNOB_MODE_VOLUME:
			if (knob_value < 1) return 0;
			if (knob_value < 10) return 1;
			if (knob_value < 19) return 2;
			if (knob_value < 28) return 3;
			if (knob_value < 37) return 4;
			if (knob_value < 46) return 5;
			if (knob_value < 55) return 6;
			if (knob_value < 64) return 7;
			if (knob_value < 72) return 8;
			if (knob_value < 81) return 9;
			if (knob_value < 90) return 10;
			if (knob_value < 99) return 11;
			if (knob_value < 108) return 12;
			if (knob_value < 117) return 13;
			if (knob_value < 127) return 14;
			return 15;

		case APC40_KNOB_MODE_PAN:
			if (knob_value < 9) return -7;
			if (knob_value < 18) return -6;
			if (knob_value < 27) return -5;
			if (knob_value < 36) return -4;
			if (knob_value < 45) return -3;
			if (knob_value < 54) return -2;
			if (knob_value < 63) return -1;
			if (knob_value < 65) return 0;
			if (knob_value < 74) return 1;
			if (knob_value < 83) return 2;
			if (knob_value < 92) return 3;
			if (knob_value < 101) return 4;
			if (knob_value < 110) return 5;
			if (knob_value < 119) return 6;
			return 7;
		}

		return 0;
	}

	bool SetKnobValueLEDCount(int type, int knob_mode, int count) // Only for Volume and Pan. Pan ranges from -7 to 7
	{
		constexpr int volume_values[16] = { 0, 5, 14, 23, 32, 41, 50, 59, 68, 76, 85, 94, 103, 112, 121, 127 };
		constexpr int pan_values[15] = { 4, 13, 22, 31, 40, 59, 58, 63, 69, 78, 87, 96, 105, 114, 123 };

		switch (knob_mode)
		{
		case APC40_KNOB_MODE_VOLUME:
			if (count < 0)
				count = 0;
			else if (count > 15)
				count = 15;
			this->SetKnobValue(type, volume_values[count]);
			return true;

		case APC40_KNOB_MODE_PAN:
			count += 7;
			if (count < 0)
				count = 0;
			else if (count > 14)
				count = 14;
			this->SetKnobValue(type, pan_values[count]);
			return true;
		}

		return false;
	}

	void SetKnobValueScaled(int type, int knob_mode, int min, int max, int value, int min_value, int max_value)
	{
		int knob_value;
		int led_count;

		switch (knob_mode)
		{
		case APC40_KNOB_MODE_SINGLE:
			knob_value = (value - min_value) * 127 / (max_value - min_value);
			SetKnobValue(type, knob_value);
			break;

		case APC40_KNOB_MODE_VOLUME:
		case APC40_KNOB_MODE_PAN:
			knob_value = (value - min_value) * 127 / (max_value - min_value);
			led_count = GetKnobValueLEDCount(knob_value, knob_mode);

			if (led_count < min)
				led_count = min;
			else if (led_count > max)
				led_count = max;

			SetKnobValueLEDCount(type, knob_mode, led_count);
			break;
		}
	}

	void SetKnobValueScaled(int type, int knob_mode, int min, int max, float value, float min_value, float max_value)
	{
		int knob_value;
		int led_count;

		switch (knob_mode)
		{
		case APC40_KNOB_MODE_SINGLE:
			knob_value = (int)((value - min_value) * 127.0f / (max_value - min_value));
			SetKnobValue(type, knob_value);
			break;

		case APC40_KNOB_MODE_VOLUME:
		case APC40_KNOB_MODE_PAN:
			knob_value = (int)((value - min_value) * 127.0f / (max_value - min_value));
			led_count = GetKnobValueLEDCount(knob_value, knob_mode);

			if (led_count < min)
				led_count = min;
			else if (led_count > max)
				led_count = max;

			SetKnobValueLEDCount(type, knob_mode, led_count);
			break;
		}
	}

	void HandleKnobValueScaled(int type, int knob_value, int knob_mode, int min, int max, int& output, int min_value, int max_value) // min/max are LED counts!
	{
		switch (knob_mode)
		{
		case APC40_KNOB_MODE_SINGLE:
			output = GetKnobValueScaled(knob_value, min_value, max_value);
			SetKnobValue(type, knob_value);
			break;

		case APC40_KNOB_MODE_VOLUME:
		case APC40_KNOB_MODE_PAN:
			int led_count = GetKnobValueLEDCount(knob_value, knob_mode);

			output = (max_value - min_value) * led_count / (max - min);
			//output = (int)((float)(max_value - min_value) * ((float)led_count / (float)(max - min)));

			if (led_count < min || output < min_value)
			{
				output = min_value;
				led_count = min;
				SetKnobValueLEDCount(type, knob_mode, led_count);
			}
			else if (led_count > max || output > max_value)
			{
				output = max_value;
				led_count = max;
				SetKnobValueLEDCount(type, knob_mode, led_count);
			}
			else
			{
				SetKnobValue(type, knob_value);
			}
		}
	}

	void HandleKnobValueScaled(int type, int knob_value, int knob_mode, int min, int max, float& output, float min_value, float max_value) // min/max are LED counts!
	{
		switch (knob_mode)
		{
		case APC40_KNOB_MODE_SINGLE:
			output = GetKnobValueScaled(knob_value, min_value, max_value);
			SetKnobValue(type, knob_value);
			break;

		case APC40_KNOB_MODE_VOLUME:
		case APC40_KNOB_MODE_PAN:
			int led_count = GetKnobValueLEDCount(knob_value, knob_mode);

			output = (max_value - min_value) * ((float)led_count / (float)(max - min));

			if (led_count < min || output < min_value)
			{
				output = min_value;
				led_count = min;
				SetKnobValueLEDCount(type, knob_mode, led_count);
			}
			else if (led_count > max || output > max_value)
			{
				output = max_value;
				led_count = max;
				SetKnobValueLEDCount(type, knob_mode, led_count);
			}
			else
			{
				SetKnobValue(type, knob_value);
			}
		}
	}

	bool PopError(APC40ErrorCode& errcode, char* text, size_t size)
	{
		if (m_vecErrorList.size() == 0)
			return false;

		errcode = m_vecErrorList[0].eErrCode;
		
		text[0] = 0;
		strcat_s(text, size, m_vecErrorList[0].szText);

		delete[] m_vecErrorList[0].szText;

		m_vecErrorList.erase(m_vecErrorList.begin());

		return true;
	}
};
