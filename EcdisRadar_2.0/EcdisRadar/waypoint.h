#ifndef WAYPOINT_H_
#define WAYPOINT_H_

#include <string>
#include "longlat.h"

class Waypoint
{
    public:
        Waypoint();
        ~Waypoint();
        std::string name;
        LongLat lon, lat;
};

#endif
