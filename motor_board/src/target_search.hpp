#ifndef TARGET_SEARCH_HPP
#define TARGET_SEARCH_HPP

#include "movement.hpp"

struct TargetCoords {

    double x;
    double y;
};

enum SearchState {
    SEARCH_ROTATE,
    SEARCH_MOVE, 
    SEARCH_DETECTED
};

void calculateCoords(const TargetData target, const RobotDriver myRobot, TargetCoords *coords);

#endif
