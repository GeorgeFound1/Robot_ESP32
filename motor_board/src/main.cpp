#include <Arduino.h>
#include "movement.hpp"
#include "pin_modes.hpp"
#include "interrupts.hpp"

volatile long leftTicks = 0;
volatile long rightTicks = 0;

void setup() {
  Serial.begin(115200);
  
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  ledcSetup(pwmChannelLeft, pwmFreq, pwmResolution);
  ledcSetup(pwmChannelRight, pwmFreq, pwmResolution);
  
  ledcAttachPin(wirePWMA, pwmChannelLeft);
  ledcAttachPin(wirePWMB, pwmChannelRight);

  pinMode(encoderLeftA, INPUT_PULLUP);
  pinMode(encoderLeftB, INPUT_PULLUP);
  pinMode(encoderRightA, INPUT_PULLUP);
  pinMode(encoderRightB, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderLeftA), readLeftEncoder, RISING);
  attachInterrupt(digitalPinToInterrupt(encoderRightA), readRightEncoder, RISING);

  delay(1000);
}


void loop() {
  Serial.println("ЖДЕМ 10 СЕКУНД....");
  static RobotDriver myRobot;
  delay(1000);

  Serial.println("======= ПРОВЕРКА ДВИЖЕНИЯ ПО КООРДИНАТАМ =======");

  myRobot.goToCoords(100, 0);
  delay(1000);
  myRobot.goToCoords(-12, 13);
  delay(1000);
  myRobot.goToCoords(36, -12);
  delay(1000);
  myRobot.goToCoords(42, 42);
  delay(1000);
  myRobot.goToCoords(21, -11);
  delay(1000);  
  myRobot.goToCoords(10, -100);
  delay(1000);
  myRobot.goToCoords(12, -13);
  delay(1000);
  myRobot.goToCoords(66, 63);
  delay(1000);
  myRobot.goToCoords(-42, -42);
  delay(1000);
  myRobot.goToCoords(-21, 11);
  delay(1000); 
  myRobot.goToCoords(0, 0);

  Serial.println("Ожидание 20 секунд перед следующим кругом...");
  
  delay(20000); 
}