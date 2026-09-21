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
    static TargetData lastTarget = {false, 0.0, 0.0};
    static bool finalTurnDone = false;
    static unsigned long lastSeenTime = 0;

    static bool isCorrectingAngle = false;
    static unsigned long correctionStartTime = 0;

    const unsigned long LOST_GRACE_PERIOD = 2000;

    float obstacleDistance = myRobot.getDistance();
    updateTargetData();

    static unsigned long lastLog = 0;
    if (millis() - lastLog > 200) {
        lastLog = millis();
        Serial.printf(
            "[LOG] Sonar: %.1f | Cam: %d | Dist: %.1f | Ang: %.1f\n",
            obstacleDistance,
            currentTarget.detected,
            currentTarget.distance,
            currentTarget.angle
        );
    }

    if (myRobot.isBusy()) {
        myRobot.update(obstacleDistance);
        return;
    }

    bool targetInWorkZone = currentTarget.detected &&
                            currentTarget.distance > 0.0 &&
                            currentTarget.distance <= TARGET_MAX_TURN_DISTANCE;

    if (targetInWorkZone) {
        lastSeenTime = millis();
        lastTarget = currentTarget;

        if (currentTarget.distance > TARGET_STOP_DISTANCE + 5.0) {
            finalTurnDone = false;
        }
    }

    if (isCorrectingAngle) {
        if (millis() - correctionStartTime < 1000) {
            myRobot.stop();
            return;
        } else {
            isCorrectingAngle = false;
        }
    }

    if (!targetInWorkZone && millis() - lastSeenTime > LOST_GRACE_PERIOD) {
        finalTurnDone = false;
        myRobot.searchTarget(obstacleDistance);
        return;
    }

    if (targetInWorkZone && currentTarget.distance <= TARGET_STOP_DISTANCE) {
        myRobot.stop();

        if (!finalTurnDone) {
            finalTurnDone = true;

            Serial.printf(
                "-> ФИНАЛЬНЫЙ ПОВОРОТ на %.1f град\n",
                currentTarget.angle
            );

            myRobot.letTurn(currentTarget.angle);
            lastSeenTime = millis();
        }

        return;
    }

    if (!targetInWorkZone && lastTarget.detected && !finalTurnDone) {
        if (lastTarget.distance <= TARGET_STOP_DISTANCE + 10.0 &&
            millis() - lastSeenTime < 1000) {

            finalTurnDone = true;

            Serial.printf(
                "-> Цель потеряна у финиша, доворот на %.1f град\n",
                lastTarget.angle
            );

            myRobot.letTurn(lastTarget.angle);
            lastSeenTime = millis();

            return;
        }
    }

    if (targetInWorkZone &&
        currentTarget.distance > TARGET_STOP_DISTANCE &&
        fabs(currentTarget.angle) > TARGET_SAFE_ANGLE) {

        myRobot.stop();

        double turnAngle = currentTarget.angle * 0.7;

        Serial.printf(
            "-> Коррекция курса: поворот на %.1f град (цель была %.1f)\n",
            turnAngle,
            currentTarget.angle
        );

        myRobot.letTurn(turnAngle);

        isCorrectingAngle = true;
        correctionStartTime = millis();
        lastSeenTime = millis();

        return;
    }

    if (targetInWorkZone && currentTarget.distance > TARGET_STOP_DISTANCE) {

        if (obstacleDistance > 0.0 && obstacleDistance <= 25.0) {
            myRobot.stop();
            return;
        }

        double d = currentTarget.distance - TARGET_STOP_DISTANCE;

        if (d > 20.0) {
            d = 20.0;
        }

        if (d > 2.0) {
            Serial.printf("-> Подъезд к цели, участок %.1f см\n", d);
            myRobot.startGoStraight(d);
        }

        return;
    }

    if (!targetInWorkZone && millis() - lastSeenTime < LOST_GRACE_PERIOD) {
        myRobot.stop();
        return;
    }
}