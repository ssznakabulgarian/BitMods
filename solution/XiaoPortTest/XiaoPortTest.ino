#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#define CYCLES_PER_MICRO_SECOND 48

byte servoPinNumber = 2;
TcCount16* const servoTimer = (TcCount16*)TC3;
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

    //enable usage of timers (TC2 and TC3)
    REG_GCLK_CLKCTRL = (unsigned short)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3));

    //await registry changes
    while (GCLK->STATUS.bit.SYNCBUSY) {}

    //disable the timers while configuring them
    servoTimer->CTRLA.reg &= ~TC_CTRLA_ENABLE;

    //set the mode, wavegen and prescaler
    servoTimer->CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV16;
    servoTimer->CC[0].reg = 46875;

    while (servoTimer->STATUS.bit.SYNCBUSY) {}

    //reset interrupts register
    servoTimer->INTENSET.reg = 0;
    servoTimer->INTENSET.bit.MC0 = 1;

    NVIC_EnableIRQ(TC3_IRQn);

    servoTimer->CTRLA.reg |= TC_CTRLA_ENABLE;
    while (servoTimer->STATUS.bit.SYNCBUSY) {}

    servoTimer->CTRLBSET.reg |= TC_CTRLBSET_CMD_STOP;
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
}

void loop() {
    int value = Serial.parseInt();
    if (value > 0) {
        setServoValue(value);
        Serial.print("Set: ");
        Serial.println(value);
    }
    else {
        Serial.println(pulseCount);
    }
}

