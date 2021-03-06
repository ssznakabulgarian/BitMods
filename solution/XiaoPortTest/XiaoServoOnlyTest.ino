#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define CYCLES_PER_MICRO_SECOND 48

byte servoPinNumber = 2;
TcCount16* const servoTimer = (TcCount16*)TC3;
TcCount16* const responseTimer = (TcCount16*)TC4;
TcCount16* const discoveryTimer = (TcCount16*)TC5;
bool servoRunning = false;
bool servoPinState = false;
volatile unsigned int servoUpTime = 4500;
volatile unsigned int servoDownTime = 55500;
unsigned int servoMin = 500;
unsigned int servoMax = 2500;
unsigned int servoPulse = 20000;

void initServo(byte pinNumber) {
    servoPinNumber = pinNumber;
    pinMode(servoPinNumber, OUTPUT);
    digitalWrite(servoPinNumber, LOW);

    REG_GCLK_CLKCTRL = (unsigned short)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3) );

    while (GCLK->STATUS.bit.SYNCBUSY) {}

    servoTimer->CTRLA.reg &= ~TC_CTRLA_ENABLE;


    //-----------------------------------------------------------
    servoTimer->CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV16;
    servoTimer->CC[0].reg = 46875;
    while (servoTimer->STATUS.bit.SYNCBUSY) {}
    //-----------------------------------------------------------
    servoTimer->INTENSET.reg = 0;
    servoTimer->INTENSET.bit.MC0 = 1;
    NVIC_EnableIRQ(TC3_IRQn);
    servoTimer->CTRLA.reg |= TC_CTRLA_ENABLE;
    while (servoTimer->STATUS.bit.SYNCBUSY) {}
    servoTimer->CTRLBSET.reg |= TC_CTRLBSET_CMD_STOP;
   
    REG_GCLK_CLKCTRL = (unsigned short)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5));
    responseTimer->CTRLA.reg &= ~TC_CTRLA_ENABLE;
    discoveryTimer->CTRLA.reg &= ~TC_CTRLA_ENABLE;
    //-----------------------------------------------------------
    responseTimer->CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV1024;
    responseTimer->CC[0].reg =  1000*clockCyclesPerMicrosecond();
    while (responseTimer->STATUS.bit.SYNCBUSY) {}
    //-----------------------------------------------------------
    discoveryTimer->CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV1;
    discoveryTimer->CC[0].reg = 20 * CYCLES_PER_MICRO_SECOND;
    while (discoveryTimer->STATUS.bit.SYNCBUSY) {}


    //-----------------------------------------------------------
    responseTimer->INTENSET.reg = 0;
    responseTimer->INTENSET.bit.MC0 = 1;
    NVIC_EnableIRQ(TC4_IRQn);
    responseTimer->CTRLA.reg |= TC_CTRLA_ENABLE;
    while (responseTimer->STATUS.bit.SYNCBUSY) {}
    responseTimer->CTRLBSET.reg |= TC_CTRLBSET_CMD_STOP;
    //-----------------------------------------------------------
    discoveryTimer->INTENSET.reg = 0;
    discoveryTimer->INTENSET.bit.MC0 = 1;
    NVIC_EnableIRQ(TC5_IRQn);
    discoveryTimer->CTRLA.reg |= TC_CTRLA_ENABLE;
    while (discoveryTimer->STATUS.bit.SYNCBUSY) {}
    discoveryTimer->CTRLBSET.reg |= TC_CTRLBSET_CMD_STOP;
}

void startServo() {
    if (servoRunning) return;
    servoRunning = true;
    servoPinState = false;
    servoTimer->CC[0].reg = servoDownTime;
    while (servoTimer->STATUS.bit.SYNCBUSY) {}
    servoTimer->CTRLBSET.reg |= TC_CTRLBSET_CMD_RETRIGGER;
}

void stopServo() {
    if (!servoRunning) return;
    servoRunning = false;
    servoTimer->CTRLBSET.reg |= TC_CTRLBSET_CMD_STOP;
}

void setServoTiming(unsigned int upTime, unsigned int pulseTime) {
    servoUpTime = upTime * (CYCLES_PER_MICRO_SECOND / 16);
    servoDownTime = pulseTime * (CYCLES_PER_MICRO_SECOND / 16) - servoUpTime;
}

void configServo(unsigned int min, unsigned int max, unsigned int pulse) {
    servoMin = min;
    servoMax = max;
    servoPulse = pulse;
}

/*value is in [0, 1000]*/
void setServoValue(unsigned int value) {
    unsigned int upTime = servoMin + ((servoMax - servoMin) * value) / 1000;
    setServoTiming(upTime, servoPulse);
}

ulong pulseCount = 0;
ulong pulses = 0;

void TC4_Handler() {
    if (responseTimer->INTFLAG.bit.MC0 == 1)
    {
        pulses++;
    responseTimer->INTFLAG.bit.MC0 = 1;
    }
}
void TC5_Handler() {}
void TC3_Handler() {
    if (servoTimer->INTFLAG.bit.MC0 == 1) {
        pulseCount++;
        if (servoPinState) {
            digitalWrite(servoPinNumber, LOW);
            servoPinState = false;
            servoTimer->CC[0].reg = servoDownTime;
            while (servoTimer->STATUS.bit.SYNCBUSY) {}
        }
        else {
            digitalWrite(servoPinNumber, HIGH);
            servoPinState = true;
            servoTimer->CC[0].reg = servoUpTime;
            while (servoTimer->STATUS.bit.SYNCBUSY) {}
        }
        servoTimer->INTFLAG.bit.MC0 = 1;
    }
}

void setup() {
    Serial.begin(9600);
    initServo(7);
    configServo(500, 2500, 20000);
    setServoValue(500);
    startServo();
    responseTimer->CTRLBSET.reg |= TC_CTRLBSET_CMD_RETRIGGER;
}

void loop() {
    if (Serial.available()){
        int value = Serial.parseInt();
        if (value > 0) {
            setServoValue(value);
            Serial.print("Set: ");
            Serial.println(value);
        }
    }
    else {
        Serial.println(servoUpTime);
    }
    Serial.printf("pulses: %d\n", pulses);
    delay(100);
}

