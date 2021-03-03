#ifndef _DEVICE_h
#define _DEVICE_h

#include "Base.h"

#define busOutLowD HIGH
#define busOutHighD LOW

#define queueLength 64

//message statuses
//  -------------------------------------------------
//  |   value   |           meaning       |
//  -------------------------------------------------
//  |   <0  |      ignoring message     |
//  -------------------------------------------------
//  |   0   |      awaiting command     |
//  -------------------------------------------------
//  |   1   |      awaiting address     |
//  -------------------------------------------------
//  |   2   |      awaiting length byte 0   |
//  -------------------------------------------------
//  |   3   |    awaiting length byte 1     |
//  -------------------------------------------------
//  |   4   |     awaiting header checksum  |
//  -------------------------------------------------
//  |   5   |    awaiting message body data |
//  -------------------------------------------------
//  |   6   | awaiting message body checksum  |
//  -------------------------------------------------
//  |   7   |    sending acknowledgement bit  |
//  -------------------------------------------------
//  |   8   |   sending response available bit  |
//  -------------------------------------------------
//  |   9   |      sending response length    |
//  -------------------------------------------------
//  |   10  |       sending response body   |
//  -------------------------------------------------
//  |   11  |     sending response checksum     |
//  -------------------------------------------------
#define MESSAGE_STATUS_IGNORED -1
#define MESSAGE_STATUS_AWAITING_COMMAND 0
#define MESSAGE_STATUS_AWAITING_ADDRESS 1
#define MESSAGE_STATUS_AWAITING_LENGTH_BYTE0 2
#define MESSAGE_STATUS_AWAITING_LENGTH_BYTE1 3
#define MESSAGE_STATUS_AWAITING_HEADER_CHECKSUM 4
#define MESSAGE_STATUS_AWAITING_BODY 5
#define MESSAGE_STATUS_AWAITING_BODY_CHECKSUM 6
#define MESSAGE_STATUS_SENDING_ACK_BIT 7
#define MESSAGE_STATUS_SENDING_RESPONSE_AVAILABLE_BIT 8
#define MESSAGE_STATUS_SENDING_RESPONSE_LENGTH 9
#define MESSAGE_STATUS_SENDING_RESPONSE_BODY 10
#define MESSAGE_STATUS_SENDING_RESPONSE_CHECKSUM 11

#define hasLengthByte0 ( firstByte & B01000000 )
#define hasLengthByte1 ( firstByte & B00100000 )
extern byte bodyData[maxMessageSizeInBytes];
extern uint bodyDataIndex;
extern byte responseLength;
extern byte response[maxResponseSizeInBytes];

//methods
void InitializeDevice(uint32_t busProbePin, uint32_t busOutPin);
void ResetDevice();
void DeviceOnBusFalling();
void DeviceOnBusRising();
void ComputeResponse(byte command, uint messageDataLength, byte* messageData, byte& responseLength, byte* response);


#endif
