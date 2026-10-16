#pragma once
#include "game/RiverCourse.h"

namespace RiverRaid
{
    constexpr int ObjectScore(ObjectKind kind)
    {
        switch (kind)
        {
        case ObjectKind::Ship: return 30;
        case ObjectKind::Helicopter: return 60;
        case ObjectKind::Plane: return 100;
        case ObjectKind::Fuel: return 80;
        case ObjectKind::Bridge: return 500;
        default: return 0;
        }
    }
}
