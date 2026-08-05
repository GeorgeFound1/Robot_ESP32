#ifndef PIN_MODES_HPP
#define PIN_MODES_HPP

#define RX_PIN 4

// Left motor
const int AIN1 = 17;
const int AIN2 = 16;
const int wirePWMA = 22;
const int encoderLeftA = 32;
const int encoderLeftB = 33;

// Right motor
const int BIN1 = 18;
const int BIN2 = 19;
const int wirePWMB = 23;
const int encoderRightA = 34;
const int encoderRightB = 35;


const int pwmChannelLeft = 0;  // Виртуальный канал 0
const int pwmChannelRight = 1; // Виртуальный канал 1
const int pwmFreq = 5000;      // 5 кГц
const int pwmResolution = 8;   // 8 бит (0-255)


const double fromTicksToCM = 51.42; // перевод тиков в сантиметры 
const double baseLenght = 15.85; // расстояние между серединами колес
extern volatile long leftTicks;
extern volatile long rightTicks;

#endif