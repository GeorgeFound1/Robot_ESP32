#include <Arduino.h>
#include "movement.hpp"
#include "pin_modes.hpp"
#include "interrupts.hpp"

// Класс для координат
class Coords {
  int x;
  int y;
};


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

  Serial.println("--- ТЕСТ ЖЕЛЕЗА ИСПРАВЛЕН ---");
  delay(1000);
}


void loop() {
  delay(10000);
  const int sideLength = 100; 
  const int turnAngle = 180; 

  Serial.println("=== НАЧАЛО ДВИЖЕНИЯ ПО КВАДРАТУ ===");

  for (int i = 0; i < 2; i++) {
    Serial.printf("Сторона %d: Едем вперед на %d см\n", i + 1, sideLength);
    goStraight(sideLength);
    
    delay(500); 

    Serial.printf("Поворот %d: Разворот на %d градусов\n", i + 1, turnAngle);
    letTurn(turnAngle);
    
    delay(500); 
  }

  Serial.println("=== КВАДРАТ ЗАВЕРШЕН ===");
  Serial.println("Ожидание 100 секунд перед следующим кругом...");
  
  delay(20000); 
}