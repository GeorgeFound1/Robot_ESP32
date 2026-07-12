#include <Arduino.h>
#include <math.h>
#include "movement.hpp"
#include "pin_modes.hpp"

void RobotDriver::setMotors(int leftSpeed, int rightSpeed) {
  // Левый мотор
  if (leftSpeed >= 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    ledcWrite(pwmChannelLeft, leftSpeed);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    ledcWrite(pwmChannelLeft, -leftSpeed);
  }

  // Правый мотор
  if (rightSpeed >= 0) {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    ledcWrite(pwmChannelRight, rightSpeed);
  } else {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    ledcWrite(pwmChannelRight, -rightSpeed);
  }
}

void RobotDriver::goStraight(const double distance) {

  leftTicks = 0;
  rightTicks = 0;

  double Kp = 7.5;
  double Ki = 0.09;
  double Kd = 0.25;

  double error = 0.0;
  double lastError = 0.0;
  double integral = 0.0;
  double derivative = 0.0;

  unsigned long lastTime = millis();
  
  long targetTicks = distance * fromTicksToCM; 

  const int baseSpeed = 180;

  while (leftTicks < targetTicks && rightTicks < targetTicks) {

    unsigned long currentTime = millis();
    double dt = (currentTime - lastTime) / 1000.0;

    if (dt == 0) {
      continue;
    }

    error = leftTicks - rightTicks;
    integral += error * dt;
    integral = constrain(integral, -500, 500);
    derivative = (error - lastError) / dt;

    double controlOutput = Kp * (error) + Ki * integral + Kd * derivative;
    int leftSpeed = baseSpeed - (int)controlOutput;
    int rightSpeed = baseSpeed + (int)controlOutput; 

    leftSpeed  = constrain(leftSpeed, 0, 255);
    rightSpeed = constrain(rightSpeed, 0, 255);

    setMotors(leftSpeed, rightSpeed);

    //Serial.printf("Ticks: LEFT >> %d ||| RIGHT >> %d\n", leftTicks, rightTicks);

    lastError = error;
    lastTime = currentTime;

    delay(10);
  }
  setMotors(0, 0);
  Serial.printf("Ticks: LEFT >> %0.2f ||| RIGHT >> %0.2f\n", (double)leftTicks / fromTicksToCM,  (double)rightTicks / fromTicksToCM);
  delay(10);
}

void RobotDriver::letTurn(const double angle) {
  leftTicks = 0;
  rightTicks = 0;

  double Kp = 0.8;
  double Kd = 0.0;
  double Kp_sync = 1.5;

  double error = 0.0;
  double lastError = 0.0;
  double derivative = 0.0;
  int outputSpeed = 0;

  unsigned long lastTime = millis();
  unsigned long currentTime = 0;
  double dt = 0;

  double targetDistance = (abs(angle) / 360.0) * (3.1415 * baseLenght);
  long targetTicks = targetDistance * fromTicksToCM;

  long currentTicks = 0;

  while (currentTicks < targetTicks) {

    currentTicks = (abs(leftTicks) + abs(rightTicks)) / 2;
    error = targetTicks - currentTicks;

    currentTime = millis();
    dt = (double)(currentTime - lastTime) / 1000.0;

    if (dt == 0) {
      delay(1);
      continue;
    }
    derivative = (error - lastError) / dt;

    outputSpeed = (int)(Kp * error + Kd * derivative);
    outputSpeed = constrain(outputSpeed, 45, 180);

    double syncError = abs(leftTicks) - abs(rightTicks);

    int leftSpeed = outputSpeed - (int)(Kp_sync * syncError);
    int rightSpeed = outputSpeed + (int)(Kp_sync * syncError);

    leftSpeed = constrain(leftSpeed, 45, 180);
    rightSpeed = constrain(rightSpeed, 45, 180);

    if (angle > 0) {
      setMotors(leftSpeed, -rightSpeed);
    } else {
      setMotors(-leftSpeed, rightSpeed);
    }
    
    lastError = error;
    lastTime = currentTime;

  }

  setMotors(0, 0);
  Serial.printf("Ticks: LEFT >> %ld ||| RIGHT >> %ld\n", leftTicks, rightTicks);
  delay(10);
}

void RobotDriver::goToCoords(const double x1, const double y1, Coords *currenrCoord) {

    double x0 = currenrCoord->x;
    double y0 = currenrCoord->y;
    double angle0 = currenrCoord->angle;

    Serial.printf("Начальная точка: x = %.2f, y = %.2f\n", x0, y0);

    double distance = sqrt(pow((x1 - x0), 2) + pow((y1 - y0), 2));
    double angle = atan2(y1 - y0, x1 - x0) * 180 / PI; // считаем угол вектора начало которого в currentCoord 
    double targetAngle = angle - angle0;
    while (targetAngle > 180) targetAngle -= 360;
    while (targetAngle < -180) targetAngle += 360;

    Serial.printf("Поворачиваем на %.2f угол\n", targetAngle);

    letTurn(targetAngle);
    goStraight(distance);
    currenrCoord->x = x1;
    currenrCoord->y = y1;
    currenrCoord->angle = angle;
    Serial.printf("Приехали в: x = %.2f, y = %.2f\n", x1, y1);
}
