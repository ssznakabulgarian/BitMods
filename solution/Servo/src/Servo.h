#include "Base.h"

byte servoPinNumber;
TcCount16* const servoTimer = (TcCount16*)TC3;
bool servoRunning = false;
bool servoPinState = false;
volatile unsigned int servoUpTime = 4500;
volatile unsigned int servoDownTime = 55500;
unsigned int servoMin = 500;
unsigned int servoMax = 2500;
unsigned int servoPulse = 20000;

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

void configServo(unsigned int min, unsigned int max, unsigned int pulse) {
    servoMin = min;
    servoMax = max;
    servoPulse = pulse;
}

/*value is in [0, 512]*/
void setServoValue(unsigned int value) {
    servoUpTime = (servoMin + ((servoMax - servoMin) * (value % 256)) / 256) * (CYCLES_PER_MICRO_SECOND / 16);
    servoDownTime = servoPulse * (CYCLES_PER_MICRO_SECOND / 16) - servoUpTime;
}

void InitializeServo(byte pinNumber) {
    servoPinNumber = pinNumber;
    pinMode(servoPinNumber, OUTPUT);
    digitalWrite(servoPinNumber, LOW);

    REG_GCLK_CLKCTRL = (unsigned short)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TCC2_TC3));
    while (GCLK->STATUS.bit.SYNCBUSY) {}
    servoTimer->CTRLA.reg &= ~TC_CTRLA_ENABLE;
    servoTimer->CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV16;
    servoTimer->CC[0].reg = 46875;
    while (servoTimer->STATUS.bit.SYNCBUSY) {}
    servoTimer->INTENSET.reg = 0;
    servoTimer->INTENSET.bit.MC0 = 1;
    NVIC_EnableIRQ(TC3_IRQn);
    servoTimer->CTRLA.reg |= TC_CTRLA_ENABLE;
    while (servoTimer->STATUS.bit.SYNCBUSY) {}
    servoTimer->CTRLBSET.reg |= TC_CTRLBSET_CMD_STOP;

    setServoValue(128);
    startServo();
}

void TC3_Handler() {
    if (servoTimer->INTFLAG.bit.MC0 == 1) {
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

