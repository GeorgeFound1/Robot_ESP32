#include <Arduino.h>
#include "movement.hpp"
#include "pin_modes.hpp"
#include "interrupts.hpp"
#include "protocol.hpp"
#include "target_search.hpp"

volatile long leftTicks = 0;
volatile long rightTicks = 0;

volatile unsigned long echoStartTime = 0;
volatile unsigned long echoDuration = 0;
volatile bool newReadingAvailable = false;

#define TIME_TO_GET_PACKET 500

void setup() {
  Serial.begin(115200);

  Serial2.begin(115200, SERIAL_8N1, RX_PIN, -1);
  
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

  pinMode(ECHO, INPUT);
  pinMode(TRIG, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(ECHO), readUltrasonic, CHANGE);

  delay(10000);
}


void loop() {
  static RobotDriver myRobot;
  static TargetCoords coordOfTarget;
  static unsigned long lastSeenCloseTime = 0;
  const unsigned long LOST_GRACE_PERIOD = 2000; // мс, сколько ждём перед повторным поиском

  float obstacleDistance = myRobot.getDistance();
  updateTargetData();

  static unsigned long lastLog = 0;
  if (millis() - lastLog > 200) {
    lastLog = millis();
    Serial.printf("[LOG] Sonar: %.1f cm | CamDetected: %d | CamDist: %.1f | CamAngle: %.1f\n",
                  obstacleDistance, currentTarget.detected, currentTarget.distance, currentTarget.angle);
  }

  if (myRobot.isBusy()) {
    myRobot.update(obstacleDistance); // один шаг движения вперёд, без блокировки loop()
    return;
  }

  if (currentTarget.detected) {
    lastSeenCloseTime = millis(); // цель видна прямо сейчас — обновляем метку

    if (currentTarget.distance > 35.0) {
      //Serial.println("-> Режим: Движение к цели (goToCoords)");
      //Serial.printf("-> ЕДЕМ, dist=%.1f\n", currentTarget.distance);
      calculateCoords(currentTarget, myRobot, &coordOfTarget);
      myRobot.goToCoords(coordOfTarget.x, coordOfTarget.y);
    } else {
      //Serial.printf("-> СТОП, dist=%.1f\n", currentTarget.distance);
      myRobot.stop();
      //Serial.println("-> Режим: Цель ближе 30 см (Стоп)");
    }
  } else {
    // Цель не видна прямо сейчас, но недавно была рядом — не срываемся в поиск сразу
    if (millis() - lastSeenCloseTime < LOST_GRACE_PERIOD) {
      myRobot.stop();
      //Serial.println("-> Цель временно не видна, ждём (grace period)");
    } else {
      //Serial.println("-> Режим: Поиск цели (searchTarget)");
      myRobot.searchTarget(obstacleDistance);
    }
  }
}