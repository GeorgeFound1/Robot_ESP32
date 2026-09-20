#ifndef MOVEMENT_HPP
#define MOVEMENT_HPP

#include <Arduino.h>
#include "pin_modes.hpp"

struct Coords {
    double x = 0.0;
    double y = 0.0;
    double angle = 0.0;
};

struct TargetData {
    bool detected;
    double distance;
    double angle;
};

class RobotDriver {
public:
    RobotDriver(double startX = 0.0, double startY = 0.0, double startAngle = 0.0) {
        currentCoords.x = startX;
        currentCoords.y = startY;
        currentCoords.angle = startAngle;
    }

    void goToCoords(const double x1, const double y1);
    void searchTarget(float obstacleDistance);

    void update(float obstacleDistance); // вызывать каждый loop(), когда isBusy()
    bool isBusy() const { return motionState != MotionState::GOING_STRAIGHT ? false : true; }

    Coords getCoords() const { return currentCoords; }
    float getDistance();
    void stop() { setMotors(0, 0); motionState = MotionState::IDLE; }

private:
    Coords currentCoords;

    enum class MotionState { IDLE, GOING_STRAIGHT };
    MotionState motionState = MotionState::IDLE;

    // персистентное состояние неблокирующего goStraight (было локальными переменными в while)
    long gsStartLeft = 0;
    long gsStartRight = 0;
    long gsTargetTicks = 0;
    double gsIntegral = 0.0;
    double gsLastError = 0.0;
    unsigned long gsLastTime = 0;

    void updateOdometry();
    void setMotors(int leftSpeed, int rightSpeed);
    void letTurn(const double angle); // без изменений, остаётся блокирующим

    void startGoStraight(const double distance);
    void stepGoStraight(float obstacleDistance);
    void goStraightBlocking(const double distance); // для searchTarget()
};

#endif