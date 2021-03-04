volatile bool responseBit = false; //used inside interrupts to store the response

uint responseDataLength;
byte responseData[maxResponseSizeInBytes];
byte responseDataChecksum = 0;
uint ControllerBusOutPin;
uint ControllerBusProbePin;

bool addressAvailable[256]; //B00000000 and B11111111 are not valid device addresses

Device* deviceList[256];
int deviceListNextIndex = 0;

ulong targetTime = 0;

void ControllerOnBusRising() {
    responseBit = true;
}

bool ReceiveBit()
{
    while (!((targetTime - micros()) & 0x80000000)) {}
    responseBit = false;
#ifdef TESTING_MODE
    DeviceOnBusFalling();
#else
    digitalWrite(ControllerBusOutPin, busOutLowC);
#endif
    targetTime += responseBitLowStateLength;
    while (!((targetTime - micros()) & 0x80000000)) {}
    bool result = responseBit;
#ifdef TESTING_MODE
    DeviceOnBusRising();
#else
    digitalWrite(ControllerBusOutPin, busOutHighC);
#endif
    targetTime += bitLength - responseBitLowStateLength;
    return result;
}

byte ReceiveByte() {
    byte result = 0;
    for (int i = 0; i < 8; i++)
    {
        result <<= 1;
        result |= ReceiveBit();
    }
    LOG(6, result);
    return result;
}

void SendBit(bool bit)
{
    if (!bit)
    {
        targetTime += bitLength;
        return;
    }

    while (!((targetTime - micros()) & 0x80000000)) {}
#ifdef TESTING_MODE
    DeviceOnBusFalling();
#else
    digitalWrite(ControllerBusOutPin, busOutLowC);
#endif
    targetTime += bitLowStateLength;
    while (!((targetTime - micros()) & 0x80000000)) {}
#ifdef TESTING_MODE
    DeviceOnBusRising();
#else
    digitalWrite(ControllerBusOutPin, busOutHighC);
#endif
    targetTime += bitHighStateLength;
}

void SendByte(byte b)
{
    LOG(4, b);
    for (int i = 0; i < 8; i++)
    {
        SendBit(b & (1 << (7 - i)));
    }
    SendBit(1);
}

bool SendMessage(byte command, byte address, uint length = 0, byte* data = 0, int attempts = 2)
{
    //setup header
    length &= 0xffff; //discard all bits to the left of the sixteenth
    command |= B10000000; //set mandatory starting bit
    byte lengthBytes[2] = { 0,0 };
    byte headerChecksum = 0;
    if (length >> 8) {
        lengthBytes[0] = length >> 8;
        headerChecksum ^= lengthBytes[0];
        command |= B01000000; //set first length info bit
    }
    if (length & 0xff) {
        lengthBytes[1] = length & 0xff;
        headerChecksum ^= lengthBytes[1];
        command |= B00100000; //set second length info bit
    }
    headerChecksum ^= command;
    headerChecksum ^= address;

    //setup body
    byte bodyChecksum = 0;
    for (uint i = 0; i < length; i++) bodyChecksum ^= data[i];

    ulong now = micros();
    if (now > targetTime) targetTime = now;

    //send message
    SendByte(command);
    SendByte(address);
    if (command & B01000000) SendByte(lengthBytes[0]);
    if (command & B00100000) SendByte(lengthBytes[1]);
    SendByte(headerChecksum);
    for (uint i = 0; i < length; i++) SendByte(data[i]);
    if (length) SendByte(bodyChecksum);

    bool success = false;
    //get ACK bit
    targetTime += delayBeforeACK;
    if (ReceiveBit())
    {
        //get response bit
        if (ReceiveBit())
        {
            //reset checksum
            responseDataChecksum = 0;

            //get response length
            responseDataLength = ReceiveByte();
            responseDataChecksum ^= responseDataLength;

            //get response data
            for (uint i = 0; i < responseDataLength; i++)
            {
                responseData[i] = ReceiveByte();
                responseDataChecksum ^= responseData[i];
            }

            //check response checksum
            responseDataChecksum ^= ReceiveByte();
            LOG(7, responseDataChecksum);
            if (responseDataChecksum == 0) {
                success = true;
            }
        }
        else
        {
            success = true;
            responseDataLength = 0;
        }
    }

    targetTime += messageTimeoutTime + bitLength;
    #ifdef DEBUG_QUEUE
    if (success) {
        Serial.println("finished send message func");
    }
    else {
        Serial.println("failed send message func");
    }
    #endif

    /*if (!success && attempts) {
        return SendMessage(command, address, length, data, attempts - 1);
    }*/

    return success;
}

bool GetDeviceInfo(byte address, Device* output)
{
    if (SendMessage(COMMAND_GET_INFO, address))
    {
        if (responseDataLength == 2 && output != nullptr)
        {
            //process the device info
            output->address = address;
            output->type = responseData[0];
            output->hasOutput = responseData[1];
            return true;
        }
    }
    return false;
}

bool DiscoverDevice(Device* output)
{
    //select a new available address
    uint newAddress = 1;
    while (!addressAvailable[newAddress] && newAddress < 255) newAddress++;
    if (newAddress == 255) return false;
    if (SendMessage(COMMAND_DISCOVER, newAddress))
    {
        if (responseDataLength == 16)
        {
            if (GetDeviceInfo(newAddress, output))
            {
                output->address = newAddress;
                addressAvailable[newAddress] = false;
                return true;
            }
        }
    }
    return false;
}

void InitializeController(int busProbePin, int busOutPin)
{
    for (int i = 0; i < 256; i++) addressAvailable[i] = true;

    ControllerBusProbePin = busProbePin;
    ControllerBusOutPin = busOutPin;
#ifndef TESTING_MODE
    pinMode(ControllerBusOutPin, OUTPUT);
    pinMode(ControllerBusProbePin, INPUT);
    digitalWrite(ControllerBusOutPin, busOutHighC);
    attachInterrupt(digitalPinToInterrupt(ControllerBusProbePin), ControllerOnBusRising, RISING);
#endif
}
