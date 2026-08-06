#include "buzzer.h"
#include "../include/config.h"

void buzzerInit() {
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
}

static void beep(int count, int onMs, int offMs = 80) {
    for (int i = 0; i < count; i++) {
        digitalWrite(PIN_BUZZER, HIGH);
        delay(onMs);
        digitalWrite(PIN_BUZZER, LOW);
        if (i < count - 1) delay(offMs);
    }
}

void beepSuccess()     { beep(2, 120); }
void beepStop()        { beep(1, 500); }
void beepDenied()      { beep(3, 80, 60); }
void beepEnroll()      { beep(2, 200, 100); }
void beepCardAdded()   { beep(3, 100, 60); }
void beepCardRemoved() { beep(2, 350, 100); }
void beepBoot()        { beep(1, 800); }
