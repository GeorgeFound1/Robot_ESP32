#ifndef TARGET_SEARCH_HPP
#define TARGET_SEARCH_HPP

#include "movement.hpp"
#include "protocol.hpp"

struct TargetCoords {

    double x;
    double y;
};

enum SearchState { 
    DRIVE_TO_WALL_1,
    TURN_90_AFTER_WALL_1,
    DRIVE_TO_WALL_2,
    TURN_180_AT_CORNER,
    SNAKE_MOVE_FORWARD,
    SNAKE_MOVE_SIDEWARD,
    SNAKE_ROTATE_1,
    SNAKE_ROTATE_2

};

extern volatile unsigned long echoDuration;
extern volatile bool newReadingAvailable;
extern volatile unsigned long echoStartTime;

extern TargetData currentTarget;
extern unsigned long lastTargetUpdateTime;

void updateTargetData();
void resetSonarBuffer();

void calculateCoords(const TargetData& target, const RobotDriver& myRobot, TargetCoords *coords);

#endif
