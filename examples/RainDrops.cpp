/*
Test application.

Buttons:

PLAY - Toggle Rain Drops
STOP - Reset Rain Drops

MAIN PAD - Launch Rain Drop
SCENE LAUNCH - Launch Rain Drops in column

CUE LEVEL - Modify sleep time

*/
// Uncomment this to see debug messages when unknown input arrives (this could happen on modes 0x40 and 0x41)
#define APC40_ALERT_UNKNOWN_INPUT

#include <iostream>
#include <cstdlib>
#include <map>
#include <APC40Interface.h>
#include <Windows.h>

void OnAPC40Input(double deltatime, std::vector< unsigned char >* message, void*);
void SendKey(int key);

int g_nSleepTime = 80;

// --------------------------------------------------------- Rain Drops

#define MAX_RAIN_DROPS  25

struct sRainDrops
{
    bool bUsed;
    int nX;
    int nY;
};
sRainDrops g_aRainDrops[MAX_RAIN_DROPS];
bool g_bRainDropsEnabled = false;

struct sRainSplashes
{
    bool bUsed;
    int nX;
    int nRadius;
    unsigned char c;
};
sRainSplashes g_aRainSplashes[MAX_RAIN_DROPS];
bool g_aRainSplashLEDStates[8][3];

void AddRainDrop(int x, int y = 0);
void ProcessRainDrop(int id);
void ProcessAllRainDrops();
void ResetRainDrops();

void AddRainSplash(int x);
void ProcessAllRainSplashes();
void ResetRainSplashes();

void InstantStopRainDrops();

// --------------------------------------------------------- 

APC40Interface<int>* APC40 = NULL;

int main()
{
    APC40 = new APC40Interface<int>();

    APC40->SetCallbackFunc(&OnAPC40Input);
    APC40->InitDevice();
    Sleep(1000);
    APC40->ResetLEDs();

    /*APC40->SetLEDMode(APC40_MAIN_PAD, 0, 0, APC40_LED_MODE_GREEN_BLINK);
    APC40->SetLEDMode(APC40_MAIN_PAD, 1, 0, APC40_LED_MODE_RED);
    APC40->SetLEDMode(APC40_MAIN_PAD, 0, 1, APC40_LED_MODE_YELLOW);
    APC40->SetLEDMode(APC40_MAIN_PAD, 1, 1, APC40_LED_MODE_YELLOW);*/

    ResetRainDrops();
    ResetRainSplashes();

    unsigned char c = 0;

    while (1)
    {
        if (g_bRainDropsEnabled && ((int)rand() % 7) == 0)
        {
            int x = (int)rand() % 8;

            AddRainDrop(x);
        }

        ProcessAllRainDrops();
        ProcessAllRainSplashes();

        Sleep(g_nSleepTime);
    }

    std::cout << "\nRunning ... press <enter> to quit.\n";
    char input;
    std::cin.get(input);

    delete APC40;
    APC40 = NULL;

    return 0;
}

void OnAPC40Input(double deltatime, std::vector< unsigned char >* message, void*)
{
    APC40Input input = APC40->GetInputFromMIDIMessage(deltatime, message);

    if (input.pressed) // This stuff only needs key press events
    {
        switch (input.type)
        {
        case APC40_MAIN_PAD:
            /*if (input.x == 0 && input.y == 0)
                SendKey(VK_MEDIA_PLAY_PAUSE);

            if (input.x == 1 && input.y == 0)
                SendKey(VK_MEDIA_STOP);

            if (input.x == 0 && input.y == 1)
                SendKey(VK_MEDIA_PREV_TRACK);

            if (input.x == 1 && input.y == 1)
                SendKey(VK_MEDIA_NEXT_TRACK);*/

            if (input.x >= 0 && input.x <= 7 && input.y >= 0 && input.y <= 6)
                AddRainDrop((int)input.x, (int)input.y);

            if (input.x == 8 && input.y >= 0 && input.y <= 6)
            {
                for(int x = 0; x <= 7; x ++)
                    AddRainDrop(x, (int)input.y);
            }
            break;

        case APC40_KNOB_CUE_LEVEL:

            if (input.value < 64)
            {
                g_nSleepTime += (g_nSleepTime / 30 + 1);
                if (g_nSleepTime > 1000)
                    g_nSleepTime = 1000;
            }
            else
            {
                g_nSleepTime -= (g_nSleepTime / 30 + 1);
                if (g_nSleepTime < 5)
                    g_nSleepTime = 5;
            }

            std::cout << "Sleep Time: " << g_nSleepTime << "\n";

            break;

        case APC40_BUTTON_PLAY:
            g_bRainDropsEnabled = !g_bRainDropsEnabled;

            if (g_bRainDropsEnabled)
                std::cout << "Enabled Rain Drops.\n";
            else
                std::cout << "Disabled Rain Drops.\n";
            break;

        case APC40_BUTTON_STOP:
            InstantStopRainDrops();
            std::cout << "Force stopped all Rain Drops.\n";
            break;
        }
    }

    #if defined APC40_ALERT_UNKNOWN_INPUT

    if (message->size() == 3 && input.type == -1)
        std::cout << std::hex << "Unknown Input - Byte1: 0x" << (int)message->at(0) << " Byte2: 0x" << (int)message->at(1) << " Byte3: 0x" << (int)message->at(2) << "\n";

    #endif
}

void SendKey(int key)
{
    INPUT ip;

    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0; // hardware scan code for key
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    ip.ki.wVk = key; // virtual-key code
    ip.ki.dwFlags = 0; // 0 for key press
    SendInput(1, &ip, sizeof(INPUT));

    ip.ki.dwFlags = KEYEVENTF_KEYUP; // release
    SendInput(1, &ip, sizeof(INPUT));
}

// --------------------------------------------------------- Rain Drops (Test)

void AddRainDrop(int x, int y)
{
    int id = -1;

    for (int i = 0; i < MAX_RAIN_DROPS; i++)
    {
        if (!g_aRainDrops[i].bUsed)
        {
            id = i;
            break;
        }
    }

    if (id == -1)
        return;

    g_aRainDrops[id].bUsed = true;
    g_aRainDrops[id].nX = x;
    g_aRainDrops[id].nY = y - 1;
}

void ProcessRainDrop(int id)
{
    if (!g_aRainDrops[id].bUsed)
        return;

    if (g_aRainDrops[id].nY != -1)
        APC40->SetLEDMode(APC40_MAIN_PAD, g_aRainDrops[id].nX, g_aRainDrops[id].nY, APC40_LED_MODE_OFF);

    g_aRainDrops[id].nY += 1;

    if (g_aRainDrops[id].nY == 7)
    {
        g_aRainDrops[id].bUsed = false;
        AddRainSplash(g_aRainDrops[id].nX);

        return;
    }

    int mode = APC40_LED_MODE_GREEN;

    if (g_aRainDrops[id].nY < 3)
        mode = APC40_LED_MODE_RED;
    else if (g_aRainDrops[id].nY < 5)
        mode = APC40_LED_MODE_YELLOW;

    APC40->SetLEDMode(APC40_MAIN_PAD, g_aRainDrops[id].nX, g_aRainDrops[id].nY, mode);
}

void ProcessAllRainDrops()
{
    for (int i = 0; i < MAX_RAIN_DROPS; i++)
        ProcessRainDrop(i);
}

void ResetRainDrops()
{
    for (int i = 0; i < MAX_RAIN_DROPS; i++)
    {
        g_aRainDrops[i].bUsed = false;
    }
}

// ---------

void AddRainSplash(int x)
{
    int id = -1;

    for (int i = 0; i < MAX_RAIN_DROPS; i++)
    {
        if (!g_aRainSplashes[i].bUsed)
        {
            id = i;
            break;
        }
    }

    if (id == -1)
        return;

    g_aRainSplashes[id].bUsed = true;
    g_aRainSplashes[id].nX = x;
    g_aRainSplashes[id].nRadius = 0;
    g_aRainSplashes[id].c = 0;
}

void ProcessAllRainSplashes()
{
    bool led_states[8][3];

    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 3; y++)
        {
            led_states[x][y] = false;
        }
    }

    int x;
    int y;

    for (int i = 0; i < MAX_RAIN_DROPS; i++)
    {
        if (g_aRainSplashes[i].bUsed)
        {
            x = g_aRainSplashes[i].nX;
            y = (g_aRainSplashes[i].nRadius + 1) / 3;

            if (y > 2)
                y = 2;

            if (x - g_aRainSplashes[i].nRadius >= 0)
                led_states[x - g_aRainSplashes[i].nRadius][y] = true;

            if (x + g_aRainSplashes[i].nRadius < 8)
                led_states[x + g_aRainSplashes[i].nRadius][y] = true;

            if (g_aRainSplashes[i].c++ == 1)
            {
                g_aRainSplashes[i].nRadius++;
                g_aRainSplashes[i].c = 0;
            }

            if (g_aRainSplashes[i].nRadius == 8)
            {
                g_aRainSplashes[i].bUsed = false;
                continue;
            }
        }
    }

    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 3; y++)
        {
            if (led_states[x][y] != g_aRainSplashLEDStates[x][y])
            {
                APC40->SetLEDMode(APC40_MAIN_PAD, x, 7 + y, (led_states[x][y] ? APC40_LED_MODE_ON : APC40_LED_MODE_OFF));

                g_aRainSplashLEDStates[x][y] = led_states[x][y];
            }
        }
    }
}

void ResetRainSplashes()
{
    for (int i = 0; i < MAX_RAIN_DROPS; i++)
    {
        g_aRainSplashes[i].bUsed = false;
    }

    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 3; y++)
        {
            g_aRainSplashLEDStates[x][y] = false;
        }
    }
}

void InstantStopRainDrops()
{
    g_bRainDropsEnabled = false;

    ResetRainDrops();
    ResetRainSplashes();

    APC40->ResetLEDs();
}
