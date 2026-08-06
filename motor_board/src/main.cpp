#include <Arduino.h>
#include "movement.hpp"
#include "pin_modes.hpp"
#include "interrupts.hpp"
#include "protocol.hpp"
#include "target_search.hpp"

volatile long leftTicks = 0;
volatile long rightTicks = 0;

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

  delay(10000);
}


void loop() {

  static RobotDriver myRobot;

  static TargetData target;
  target.detected = false;

  static TargetCoords coordOfTarget;

  static unsigned long lastTimeFromData = 0;

  CameraData packet;
  if (dataUnpackage(&packet)) {
    dataCopy(&packet, &target);
    lastTimeFromData = millis();


  } 

  /*if(millis() - lastTimeFromData > TIME_TO_GET_PACKET) {
    myRobot.goToCoords(0, 0);
  }*/
  Serial.printf("Distance = %0.2f angle = %0.2f\n", target.distance, target.angle);

  if (target.detected) {

    calculateCoords(target, myRobot, &coordOfTarget);
    myRobot.goToCoords(coordOfTarget.x, coordOfTarget.y);

  } 
}