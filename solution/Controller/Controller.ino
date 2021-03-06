//#define MEGA 
//#define DEBUG_QUEUE

#include "ControllerBase.h"
#include "ControllerBaseImpl.h"

void setup()
{
	Serial.begin(9600);
	Serial.setTimeout(100);
	InitializeController(9, 10);
}

void loop() {
	//attempt discovery
	if (DiscoverDevice(&deviceList[deviceListNextIndex]))
	{
		Serial.printf("discovered new device: %d\n", deviceList[deviceListNextIndex].address);

		//update device list index
		deviceListNextIndex++;

		//start over
		return;
	}
	
	//check for missing devices
	for (int i = 0; i < deviceListNextIndex; i++)
	{
		if (!GetDeviceInfo(deviceList[i].address, &deviceList[i]))
		{
			Serial.printf("device at %d lost\n", deviceList[i].address);
			addressAvailable[deviceList[i].address] = true;
			deviceList[i] = deviceList[--deviceListNextIndex];
		}
		else
		{
			Serial.printf("device at %d respondeed to checkup\n", deviceList[i].address);
		}
	}
	
	//check for messages from PC
	while (Serial.available()) {
		byte command = Serial.read();
		byte address = 0;
		byte bodyLength = 0;
		byte body[256] = {0};

		switch (command) {
			case 97:
				//dump device list
				for (int i = 0; i < deviceListNextIndex; i++)
				{
					Serial.print(deviceList[i].address);
					Serial.print(deviceList[i].type);
				}
				break;
			case 98:
				//get device data
				while (!Serial.available()) {}
				address = Serial.read();
				if (SendMessage(COMMAND_GET_DATA, address))
				{
					Serial.print(responseDataLength);
					for(int i = 0; i < responseDataLength; i++)
					{
						Serial.print(responseData[i]);
					}
				}
				else
				{
					Serial.print(0);
				}
				break;
			case 99:
				//send payload to device
				Serial.readBytes((char*) &address, 1);
				Serial.readBytes((char*) &bodyLength, 1);
				
				for (int i = 0; i < bodyLength; i++)
				{
					Serial.readBytes((char*) &body[i], 1);
				}

				if (SendMessage(COMMAND_EXECUTE, address, bodyLength, body))
				{
					if(responseDataLength==1) Serial.print(responseData[0]);
				}
				else
				{
					Serial.print(0);
				}
				break;
		}
	}
	delay(3000);
}
