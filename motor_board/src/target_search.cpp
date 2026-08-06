#include <math.h>
#include "target_search.hpp"
#include "movement.hpp"

void calculateCoords(const TargetData target, const RobotDriver myRobot, TargetCoords *coords) {

    TargetCoords buffer;

    buffer.x = target.distance * cos((float)target.angle);
    buffer.y = target.angle * sin((float)target.angle);

    coords->x = buffer.x + myRobot.getCoords().x;
    coords->y = buffer.y + myRobot.getCoords().y;

    return;
}

void RobotDriver::searchTarget(const TargetData target) {

    static SearchState search = SEARCH_ROTATE;

    switch (search)
    {
    case SEARCH_ROTATE:
        while (((currentCoords.angle - 180.0) < 180.0) || !target.detected) {
            letTurn(20.0);

        }
        if (target.detected) search = SEARCH_DETECTED;
        search = SEARCH_MOVE;
        break;
    
    case SEARCH_MOVE:
        
        break;
    }

}