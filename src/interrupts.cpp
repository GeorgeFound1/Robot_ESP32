#include <Arduino.h>
#include "pin_modes.hpp"

void IRAM_ATTR readLeftEncoder() {
  // Если сигналы на А и B несовпадают, значит крутимся в одну сторону, если разные — в другую
  if (digitalRead(encoderLeftA) != digitalRead(encoderLeftB)) {
    leftTicks++;
  } else {
    leftTicks--;
  }
}

void IRAM_ATTR readRightEncoder() {
  if (digitalRead(encoderRightA) == digitalRead(encoderRightB)) {
    rightTicks++;
  } else {
    rightTicks--;
  }
}