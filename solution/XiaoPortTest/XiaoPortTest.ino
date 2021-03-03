#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

void setup() {
    Serial.begin(9600);

    pinMode(6, INPUT);
    pinMode(7, OUTPUT);
    digitalWrite(7, LOW);

    //attachInterrupt(digitalPinToInterrupt(6), DeviceOnBusFalling, FALLING);
    //attachInterrupt(digitalPinToInterrupt(busProbePin), DeviceOnBusRising, RISING);
}

ulong lastFall = 0;
ulong l[256];
volatile int s = 0; 
volatile int e = 0;

void DeviceOnBusFalling() {
    ulong n = micros();
    ulong delta = n - lastFall;
    lastFall = n;
    l[e] = delta;
    e = (e + 1) & 0xff;
    if (e == s) s = (s + 1) & 0xff;
}

bool state = 0;
void loop() {
    digitalWrite(7, state ? HIGH : LOW);
    state = !state;
    delay(1000);
    Serial.println("---");
    while (s != e) {
        Serial.println(l[s]);
        s = (s + 1) & 0xff;
    }
}
