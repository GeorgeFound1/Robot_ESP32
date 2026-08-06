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
    void searchTarget(const TargetData target);

    Coords getCoords() const { return currentCoords; }

private:
    Coords currentCoords;

    void updateOdometry();

    void setMotors(int leftSpeed, int rightSpeed);
    void goStraight(const double distance);
    void letTurn(const double angle);
};

#endif