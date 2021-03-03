#include "DeviceBase.h"

uint32_t busOutPin;
uint32_t busProbePin;
volatile bool risingTransitionOccurred;
ulong lastFallingTransitionTime = 0x80000000;
ulong delta = 0;
uint responseDataNextIndex = 0;
uint receivedBits = 0; /**< \brief number of bits received since the last complete byte */
uint sentBits = 0; /**< \brief number of bits sent since the last complete byte */
#ifdef MEGA
#else
TcCount16* const responseTimer = (TcCount16*)TC4;
TcCount16* const discoveryTimer = (TcCount16*)TC5;
#endif

byte command = 0;
byte address = 0;
uint bodyLength = 0;
byte headerChecksum = 0;
byte bodyData[maxMessageSizeInBytes] = { 0 };
uint bodyDataIndex = 0;
byte bodyDataChecksum = 0;
byte responseLength = 0;
byte response[maxResponseSizeInBytes] = { 0 };
byte responseChecksum = 0;
short messageStatus = MESSAGE_STATUS_IGNORED;
ulong lastCommandTime = 0;
byte deviceAddress = 0;
byte deviceSerialNumber[16];
bool isDeviceDiscovered = false;
byte firstByte;
byte currentByte;

void InitializeDevice(uint32_t _busProbePin, uint32_t _busOutPin)
{
    busProbePin = _busProbePin;
    busOutPin = _busOutPin;

#ifdef MEGA
    deviceSerialNumber[0] = 0;
    deviceSerialNumber[1] = 1;
    deviceSerialNumber[2] = 2;
    deviceSerialNumber[3] = 3;
    deviceSerialNumber[4] = 4;
    deviceSerialNumber[5] = 5;
    deviceSerialNumber[6] = 6;
    deviceSerialNumber[7] = 7;
    deviceSerialNumber[8] = 8;
    deviceSerialNumber[9] = 9;
    deviceSerialNumber[10] = 10;
    deviceSerialNumber[11] = 11;
    deviceSerialNumber[12] = 12;
    deviceSerialNumber[13] = 13;
    deviceSerialNumber[14] = 14;
    deviceSerialNumber[15] = 15;
#else
    uint32_t* ptr = (uint32_t*)deviceSerialNumber;
    *(ptr++) = *((uint32_t*)0x0080A00C);
    *(ptr++) = *((uint32_t*)0x0080A040);
    *(ptr++) = *((uint32_t*)0x0080A044);
    *(ptr) = *((uint32_t*)0x0080A048);
#endif

#ifndef TESTING_MODE
    //configure pins
    pinMode(busOutPin, OUTPUT);
    pinMode(busProbePin, INPUT);
    digitalWrite(busOutPin, busOutLowD);

    //set up bus interrupts
    attachInterrupt(digitalPinToInterrupt(busProbePin), DeviceOnBusFalling, FALLING);
    //attachInterrupt(digitalPinToInterrupt(busProbePin), DeviceOnBusRising, RISING);
    //TODO: fix rising interrupt issues
#endif

#ifdef MEGA
    noInterrupts();
    TCNT4 = 0;
    TCCR4A = (1 << WGM42);
    TCCR4B = 0;
    OCR4A = responseBitLength * CYCLES_PER_MICRO_SECOND;
    TIMSK4 |= (1 << OCIE4A);

    TCNT5 = 0;
    TCCR5A = (1 << WGM52);
    TCCR5B = 0;
    OCR5A = discoveryPriorityCheckTime * CYCLES_PER_MICRO_SECOND;
    TIMSK5 |= (1 << OCIE5A);
    interrupts();

#else
    //enable usage of timers (TC4 and TC5)
    REG_GCLK_CLKCTRL = (unsigned short)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5));

    //await registry changes
    while (GCLK->STATUS.bit.SYNCBUSY) {}

    //disable the timers while configuring them
    responseTimer->CTRLA.reg &= ~TC_CTRLA_ENABLE;
    discoveryTimer->CTRLA.reg &= ~TC_CTRLA_ENABLE;

    //set the mode, wavegen and prescaler
    responseTimer->CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV1;
    responseTimer->CC[0].reg = responseBitLength * CYCLES_PER_MICRO_SECOND;

    while (responseTimer->STATUS.bit.SYNCBUSY) {}

    //set the compare/capture register value
    discoveryTimer->CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV1;
    discoveryTimer->CC[0].reg = discoveryPriorityCheckTime * CYCLES_PER_MICRO_SECOND;

    //await registry changes
    while (discoveryTimer->STATUS.bit.SYNCBUSY) {}

    //reset interrupts register
    responseTimer->INTENSET.reg = 0;
    responseTimer->INTENSET.bit.MC0 = 1;

    NVIC_EnableIRQ(TC4_IRQn);

    responseTimer->CTRLA.reg |= TC_CTRLA_ENABLE;
    while (responseTimer->STATUS.bit.SYNCBUSY) {}

    responseTimer->CTRLBSET.reg |= TC_CTRLBSET_CMD_STOP;

    //enable interrupt on compare channel 0 match
    discoveryTimer->INTENSET.reg = 0;
    discoveryTimer->INTENSET.bit.MC0 = 1;

    NVIC_EnableIRQ(TC5_IRQn);

    //reenable timer
    discoveryTimer->CTRLA.reg |= TC_CTRLA_ENABLE;

    //await registry changes
    while (discoveryTimer->STATUS.bit.SYNCBUSY) {}

    //stop the timer since it's not in use
    discoveryTimer->CTRLBSET.reg |= TC_CTRLBSET_CMD_STOP;
#endif
}

void ResetDevice() {
    deviceAddress = 0;
    isDeviceDiscovered = false;
    messageStatus = MESSAGE_STATUS_IGNORED;
}

byte respState = 1;

#ifdef MEGA
#define StartTimer4() respState = 0; TCNT4 = 0; TCCR4B = (1 << CS40);
#define StartTimer5() TCNT5 = 0; TCCR5B = (1 << CS50);

ISR(TIMER4_COMPA_vect) {
    TCNT4 = 0;
    if (respState == 0) {
        ++respState;
#ifndef TESTING_MODE
        digitalWrite(busOutPin, busOutHigh);
#else
        ControllerOnBusRising();
#endif
    }
    else {
#ifndef TESTING_MODE
        digitalWrite(busOutPin, busOutLow);
#endif
        TCCR4B = 0;
    }
}

ISR(TIMER5_COMPA_vect) {
    TCCR5B = 0;
    if (risingTransitionOccurred) messageStatus = MESSAGE_STATUS_IGNORED;
}

#else
#define StartTimer4() respState = 0; responseTimer->CTRLBSET.reg |= TC_CTRLBSET_CMD_RETRIGGER;
#define StartTimer5() discoveryTimer->CTRLBSET.reg |= TC_CTRLBSET_CMD_RETRIGGER;

void TC4_Handler() {
    if (responseTimer->INTFLAG.bit.MC0 == 1) {
        if (respState == 0) {
#ifndef TESTING_MODE
            digitalWrite(busOutPin, busOutHighD);
#else
            ControllerOnBusRising();
#endif
            ++respState;
        }
        else {
#ifndef TESTING_MODE
            digitalWrite(busOutPin, busOutLowD);
#endif
            responseTimer->CTRLBSET.reg |= TC_CTRLBSET_CMD_STOP;
        }
        responseTimer->INTFLAG.bit.MC0 = 1;
    }
}

void TC5_Handler() {
    if (discoveryTimer->INTFLAG.bit.MC0 == 1) {
        if (risingTransitionOccurred) messageStatus = MESSAGE_STATUS_IGNORED;
        discoveryTimer->CTRLBSET.reg |= TC_CTRLBSET_CMD_STOP;
        discoveryTimer->INTFLAG.bit.MC0 = 1;
    }
}
#endif

void DeviceOnBusRising()
{
    risingTransitionOccurred = true;
}

void DeviceOnBusFalling()
{
    const ulong now = micros();
    const ulong delta = now - lastFallingTransitionTime;
    LOG(0, delta);
    if (delta < minPulseTime) return;
    lastFallingTransitionTime = now;
    if (delta >= messageTimeoutTime){
        LOG(1, 11);
        LOG(0, delta);
        receivedBits = 1;
        messageStatus = MESSAGE_STATUS_AWAITING_COMMAND;
        currentByte = 1;
    }
    else if (messageStatus >= MESSAGE_STATUS_AWAITING_COMMAND)
    {
        byte b = 0;
        //read bits from the bus if the message isn't received
        if (messageStatus < MESSAGE_STATUS_SENDING_ACK_BIT)
        {
            //add all the preceding zeroes
            uint numberOfZeroes = (delta + halfBitLength) / bitLength - 1;
            currentByte <<= numberOfZeroes;
            receivedBits += numberOfZeroes;

            //add the newly detected one
            if (receivedBits >= 8)
            {
                if (receivedBits > 8) {
                    messageStatus = MESSAGE_STATUS_IGNORED;
                    return;
                }
                else
                {
                    b = currentByte;
                    currentByte = 0;
                    receivedBits = 0;
                    LOG(9, b);
                }
            }
            else
            {
                currentByte <<= 1;
                currentByte |= 1;
                receivedBits++;
                return;
            }
        }

        //process data
        switch (messageStatus)
        {
        case MESSAGE_STATUS_AWAITING_COMMAND:
            //reset variables
            bodyDataIndex = 0;
            bodyDataChecksum = 0;
            bodyLength = 0;
            responseDataNextIndex = 0;

            //receive command
            firstByte = b;
            headerChecksum = firstByte;
            command = firstByte & B00011111;
            if (command == COMMAND_DISCOVER && isDeviceDiscovered) messageStatus = MESSAGE_STATUS_IGNORED;
            else messageStatus = MESSAGE_STATUS_AWAITING_ADDRESS;
            break;
        case MESSAGE_STATUS_AWAITING_ADDRESS:
            //receive address
            address = b;
            headerChecksum ^= address;
            if (command == COMMAND_DISCOVER)
            {
                if (isDeviceDiscovered) messageStatus = MESSAGE_STATUS_IGNORED;
                else messageStatus = MESSAGE_STATUS_AWAITING_HEADER_CHECKSUM;
            }
            else
            {
                if (command == COMMAND_GET_INFO && address == deviceAddress && !isDeviceDiscovered) isDeviceDiscovered = true;

                if ((address != deviceAddress && address != broadcastAddress) || !isDeviceDiscovered)
                {
                    messageStatus = MESSAGE_STATUS_IGNORED;
                }
                else
                {
                    if (hasLengthByte0)
                    {
                        messageStatus = MESSAGE_STATUS_AWAITING_LENGTH_BYTE0;
                    }
                    else if (hasLengthByte1)
                    {
                        messageStatus = MESSAGE_STATUS_AWAITING_LENGTH_BYTE1;
                    }
                    else
                    {
                        messageStatus = MESSAGE_STATUS_AWAITING_HEADER_CHECKSUM;
                    }
                }
            }
            break;
        case MESSAGE_STATUS_AWAITING_LENGTH_BYTE0:
            //receive length byte 0
            headerChecksum ^= b;
            if (hasLengthByte1) messageStatus = MESSAGE_STATUS_AWAITING_LENGTH_BYTE1;
            else messageStatus = MESSAGE_STATUS_AWAITING_HEADER_CHECKSUM;

            //update body length
            bodyLength |= b << 8;
            break;
        case MESSAGE_STATUS_AWAITING_LENGTH_BYTE1:
            //receive length byte 1
            headerChecksum ^= b;
            messageStatus = MESSAGE_STATUS_AWAITING_HEADER_CHECKSUM;

            //update body length
            bodyLength |= b;
            break;
        case MESSAGE_STATUS_AWAITING_HEADER_CHECKSUM:
            //receive header checksum
            headerChecksum ^= b;
            if (headerChecksum != 0) messageStatus = MESSAGE_STATUS_IGNORED;
            else
            {
                if (bodyLength == 0) {
                    messageStatus = MESSAGE_STATUS_SENDING_ACK_BIT;
                    if (command == COMMAND_DISCOVER)
                    {
                        LOG(4, 1);
                        responseLength = 16;
                        memcpy((char*)response, (char*)deviceSerialNumber, 16); //ReSharper shows a false error
                    }
                    else
                    {
                        ComputeResponse(command, bodyLength, bodyData, responseLength, response);
                    }
                }
                else if(bodyLength > maxMessageBodySizeInBytes) messageStatus = MESSAGE_STATUS_IGNORED;
                else messageStatus = MESSAGE_STATUS_AWAITING_BODY;
            }

            break;
        case MESSAGE_STATUS_AWAITING_BODY:
            //receive message body
            bodyData[bodyDataIndex++] = b;
            bodyDataChecksum ^= b;
            if (bodyDataIndex == bodyLength) messageStatus = MESSAGE_STATUS_AWAITING_BODY_CHECKSUM;
            break;
        case MESSAGE_STATUS_AWAITING_BODY_CHECKSUM:
            //receive message body checksum
            bodyDataChecksum ^= b;
            if (bodyDataChecksum != 0) messageStatus = MESSAGE_STATUS_IGNORED;
            else
            {
                ComputeResponse(command, bodyLength, bodyData, responseLength, response);
                messageStatus = MESSAGE_STATUS_SENDING_ACK_BIT;
            }
            break;
            //time to compute given
        case MESSAGE_STATUS_SENDING_ACK_BIT:
            //send ACK bit
            LOG(4, 2);
            StartTimer4();
            messageStatus = MESSAGE_STATUS_SENDING_RESPONSE_AVAILABLE_BIT;
            break;
        case MESSAGE_STATUS_SENDING_RESPONSE_AVAILABLE_BIT:
            //send response available bit
            if (responseLength)
            {
                LOG(4, 3);
                responseChecksum = responseLength;
                StartTimer4();
                messageStatus = MESSAGE_STATUS_SENDING_RESPONSE_LENGTH;
                sentBits = 0;
            }
            else messageStatus = MESSAGE_STATUS_IGNORED;
            break;
        case MESSAGE_STATUS_SENDING_RESPONSE_LENGTH:
            //send response length byte
            if (responseLength & (B10000000 >> sentBits))
            {
                StartTimer4();
            }
            sentBits++;
            if (sentBits == 8)
            {
                LOG(4, 4);
                sentBits = 0;
                messageStatus = MESSAGE_STATUS_SENDING_RESPONSE_BODY;
            }
            break;
        case MESSAGE_STATUS_SENDING_RESPONSE_BODY:
            //send response body
            if (response[responseDataNextIndex] & (B10000000 >> sentBits))
            {
                StartTimer4();
            }
            else if (command == COMMAND_DISCOVER) //check for prioritized transmissions only during discovery messages
            {
                risingTransitionOccurred = false;
                StartTimer5();
            }

            sentBits++;
            if (sentBits == 8)
            {
                sentBits = 0;
                responseChecksum ^= response[responseDataNextIndex++];
                if (responseDataNextIndex == responseLength) {
                    LOG(4, 5);
                    messageStatus = MESSAGE_STATUS_SENDING_RESPONSE_CHECKSUM;
                }
            }
            break;
        case MESSAGE_STATUS_SENDING_RESPONSE_CHECKSUM:
            //update address if a discovery message has went through
            if (command == COMMAND_DISCOVER) deviceAddress = address;
            //send response checksum
            if (responseChecksum & (B10000000 >> sentBits))
            {
                StartTimer4();
            }
            sentBits++;
            if (sentBits == 8)
            {
                sentBits = 0;
                LOG(4, 6);
                messageStatus = MESSAGE_STATUS_IGNORED;
            }
            break;
        default:
            break;
        }
    }
}
