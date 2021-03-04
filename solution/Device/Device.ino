//#define MEGA
//#define DEBUG_QUEUE

#include "DeviceBase.h"
#include "DeviceBaseImpl.h"

void setup()
{
	//Serial.begin(9600);
	InitializeDevice(10, 9);
}

void loop()
{
	if (isDeviceDiscovered)
	{
		//reset device if no command has been received for the past 10 seconds
		if (micros() - lastCommandTime > 10000000) ResetDevice();

		//extract data from functional components if available
		//...
	}
}

void ComputeResponse(byte command, uint messageDataLength, byte* messageData, byte& responseLength, byte* response)
{
	lastCommandTime = micros();
	switch (command)
	{
	case COMMAND_GET_INFO:
		responseLength = 2;
		response[0] = DEVICE_TYPE_SERVO_180;
		response[1] = 0;
		break;
	case 5:
		responseLength = messageDataLength * 2;
		for (int i = 0; i < messageDataLength; i++) {
			response[i * 2] = messageData[i];
		}
		break;
	default:
		responseLength = 0;
		break;
	}
}