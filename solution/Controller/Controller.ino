//#define MEGA 
#define DEBUG_QUEUE

#include "ControllerBase.h"
#include "ControllerBaseImpl.h"

Device d;

void setup()
{
#ifdef DEBUG_QUEUE
	Serial.begin(9600);
#endif
	InitializeController(9,10);
}

byte sb = 0;
int speed = 90;
int sc = 10;

void loop() {
	if (d.address == 0) {
		Serial.print("Disconvering...");
		int outcome = DiscoverDevice(&d);
		Serial.print("discover:");
		Serial.println(outcome);
		speed = 90;
		sc = 1;
	}
	if (d.address != 0) {
		LOG(1, 5);
		int outcome = GetDeviceInfo(d.address, &d);
		LOG(1, 6);
		Serial.print("info:");
		Serial.println(outcome);

		responseDataLength = 0;
		byte arr[6];
		arr[0] = sb;
		arr[1] = sb + 1;
		arr[2] = sb + 2;
		arr[3] = sb + 3;
		arr[4] = sb + 4;
		arr[5] = sb + 5;
		SendMessage(5, d.address, 6, arr);

		bool ok = false;
		if (responseDataLength == 12) {
			ok = true;
			for (int i = 0; i < 6; i++) {
				if (responseData[i * 2] != arr[i]) {
					ok = false;
				}
			}
		}

		arr[0] = speed;
		SendMessage(6, d.address, 1, arr);
		speed += sc;
		if (speed >= 180) {
			speed = 180;
			sc = -1;
		}
		if (speed <= 0) {
			speed = 0;
			sc = 1;
		}

		Serial.print("ok:");
		Serial.println(ok ? "yes" : "no");

		if (!ok) {
			d.address = 0;
		}
	}

	dumpLog();
	delay(2000);
}
