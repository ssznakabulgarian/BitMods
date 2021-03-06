//#define MEGA
//#define DEBUG_QUEUE

#include "DeviceBase.h"
#include "DeviceBaseImpl.h"
#include "Servo.h"

void setup()
{
	//Serial.begin(9600);
	InitializeDevice(10, 9);
	InitializeServo(1);
}

void loop()
{
	if (isDeviceDiscovered)
	{
		//reset device if no command has been received for the past 10 seconds
		if (micros() - lastCommandTime > 9000000) ResetDevice();

		//extract data from functional components if available
		//...
	}
}

void ComputeResponse(byte command, uint messageDataLength, byte* messageData, byte& responseLength, byte* response)
{
	switch (command)
	{
	case COMMAND_EXECUTE:
		if (messageDataLength == 1)
		{
			setServoValue(messageData[0]);
			responseLength = 1;
			response[0] = 'c'; //success
		}
		else
		{
			responseLength = 1;
			response[0] = 'e'; //error
		}
		break;
	default:
		responseLength = 0;
		break;
	}
}