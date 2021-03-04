// Controller.h

#ifndef _Controller_h
#define _Controller_h

#include "Base.h"

#define busOutLowC LOW
#define busOutHighC HIGH

//message characteristics
#define delayBeforeACK (maximumZeroCount - 1) * bitLength

class Device
{
public:
    bool hasOutput;
    int type;
    byte address;
    Device()
    {
        hasOutput = false;
        type = -1;
        address = 0;
    }
    Device(bool hasOutput, byte address, int type)
    {
        this->address = address;
        this->hasOutput = hasOutput;
        this->type = type;
    }
};

void InitializeController(int busProbePin, int busOutPin);
void ControllerOnBusRising();
bool SendMessage(byte command, byte address, uint length, byte* data, int attempts);
bool GetDeviceInfo(byte address, Device* output);
bool DiscoverDevice(Device* output);

#endif
