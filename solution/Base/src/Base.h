#ifndef _BASE_h
#define _BASE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#ifndef uint
#define uint unsigned int
#endif

#ifndef ulong
#define ulong unsigned long
#endif

//clock characteristics
#ifdef MEGA
#define CYCLES_PER_MICRO_SECOND 16
#else
#define CYCLES_PER_MICRO_SECOND 48
#endif

//bus characteristics
//#define busOutLow LOW
//#define busOutHigh HIGH

//buss addresses
#define broadcastAddress B11111111

//bit characteristics
#define bitLength 1000
#define halfBitLength ( bitLength/2 )
#define bitLowStateLength ((bitLength * 20) / 100)
#define bitHighStateLength (bitLength - bitLowStateLength)
#define responseBitLength ((bitLength * 10) / 100)
#define responseBitLowStateLength (3 * responseBitLength)

//message characteristics
#define maximumZeroCount ( 8 )
#define messageTimeoutTime ((maximumZeroCount + 1) * bitLength + halfBitLength )
#define minPulseTime halfBitLength
#define maxMessageSizeInBytes 1000
#define maxMessageBodySizeInBytes (maxMessageSizeInBytes - 5)
#define maxResponseSizeInBytes 255
#define discoveryPriorityCheckTime (2 * responseBitLength)
#define queueLength 64

//message commands
#define COMMAND_DISCOVER B00000001 //discover device
#define COMMAND_GET_INFO B00000010 //get info about device (type, hasOutput)
#define COMMAND_GET_DATA B00000011 //get the latest data stored on a device with output
#define COMMAND_EXECUTE B00000100 //order execution of the instructions provided inside the message body


//types
#define DEVICE_TYPE_DC_MOTOR 0
#define DEVICE_TYPE_SERVO_360 1
#define DEVICE_TYPE_SERVO_180 2
#define DEVICE_TYPE_SERVO_CLAW 3
#define DEVICE_TYPE_TERMOMETER 4
#define DEVICE_TYPE_GYROSCOPE 5
#define DEVICE_TYPE_ACCELEROMETER 6
#define DEVICE_TYPE_LCD 7

//testing and debug stuff
#ifdef TESTING_MODE
#define DEBUG_QUEUE
#endif

#ifdef DEBUG_QUEUE
uint tqi;
ulong tq[256];
#define LOG(type, value) tq[tqi] = (type) * 1000 + (value); tqi = (tqi + 1) & 0xff;

void dumpLog() {
	Serial.println("---LOG---");
	for (uint i = 0; i < tqi; i++) {
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
	Serial.printf("at: %d\n", micros());
	Serial.println("---------");
	tqi = 0;
}

#else
#define LOG(type, value)
#endif


#endif