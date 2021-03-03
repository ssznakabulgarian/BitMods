#define TESTING_MODE
//#define MEGA

#include "DeviceBase.h"
#include "ControllerBase.h"
#include "DeviceBaseImpl.h"
#include "ControllerBaseImpl.h"
/*
 Name:		Testing.ino
 Created:	25-02-2021 23:28
 Author:	ruslan
*/

void dumpLog() {
	Serial.println("---LOG---");
	for (int i = 0; i < tqi; i++) {
		if (i > 0) {
			if ((i & 0x7) == 0) {
				Serial.println();
			}
			else {
				Serial.print(" ");
			}
		}
		Serial.print(tq[i]);
	}
	Serial.println();
	Serial.println("---------");
	tqi = 0;
}
Device d;

/*volatile long timer = 0;
volatile int n = 0;

ISR(TIMER4_COMPA_vect) {
	n = TCNT4;
	TCNT4 = 0;
	timer = micros();
	TCCR4B = 0;
}*/

// the setup function runs once when you press reset or power the board
void setup() {
/*	Serial.begin(9600);

	noInterrupts();
	TCNT4 = 0;
	TCCR4A = (1 << WGM42);
	TCCR4B = 0;
	OCR4A = 70;
	TIMSK4 |= (1 << OCIE4A);
	interrupts();

	while(true) {	
		ulong t0 = micros();
		TCNT4 = 0;
		TCCR4B = (1 << CS40);
		delay(1000);
		Serial.println(timer - t0);
		Serial.println(n);
		Serial.println(TCNT4);
		Serial.println(OCR4A);
		timer = 0;
	}

	return;*/
	Serial.begin(9600);
	LOG(1, 1);
	InitializeDevice(0, 0);
	LOG(1, 2);
	InitializeController(0, 0);
	LOG(1, 3);
	int outcome = DiscoverDevice(&d);
	LOG(1, 4);
	Serial.print("discover:");
	Serial.println(outcome);
}

// the loop function runs over and over again until power down or reset
byte sb = 0; 
void loop() {
	LOG(1, 5);
	int outcome = GetDeviceInfo(d.address, &d);
	LOG(1, 6);
	Serial.print("info:");
	Serial.println(outcome);

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

	Serial.print("ok:");
	Serial.println(outcome);

	dumpLog();
	delay(5000);
}

void ComputeResponse(byte command, uint messageDataLength, byte* messageData, byte& responseLength, byte* response) {
	if (command == COMMAND_GET_INFO) {
		responseLength = 2;
		response[0] = DEVICE_TYPE_SERVO;
		response[1] = 0;
	}
	else if (command == 5) {
		responseLength = messageDataLength * 2;
		for (int i = 0; i < messageDataLength; i++) {
			response[i * 2] = messageData[i];
		}
	}
	else {
		responseLength = 0;
	}
}
