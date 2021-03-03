// Controller.h

#ifndef _Controller_h
#define _Controller_h

#include "Base.h"

#define busOutLowC LOW
#define busOutHighC HIGH

//error codes
#define ERROR_STATUS_RESPONSE_INVALID -1
#define ERROR_SEND_MESSAGE_FAILED -2
#define ERROR_DISCOVERY_FAILED -3
#define ERROR_NO_AVAILABLE_ADDRESSES -4

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
bool SendMessage(byte command, byte address, uint length, byte* data);
int GetDeviceInfo(byte address, Device* output);
int DiscoverDevice(Device* output);

#endif
