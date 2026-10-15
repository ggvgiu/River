#pragma once
#include "RiverRaidGame.h"

// Presentation only: reads the model, submits OpenGL geometry, changes no gameplay.
class RiverRaidView
{
public:
    void Draw(const RiverRaid::RiverRaidGame& game) const;
};
