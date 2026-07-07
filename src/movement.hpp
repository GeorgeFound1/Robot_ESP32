#ifndef MOVEMENT_HPP
#define MOVEMENT_HPP

#include <Arduino.h>
#include "pin_modes.hpp"

class RobotDriver {
    public:

    void goToCoords(const double x1, const double y1, Coords *currenrCoord);

    private:

    void setMotors(int leftSpeed, int rightSpeed);
    void goStraight(const double distance);
    void letTurn(const double angle);

};

#endif