#pragma once
#include "RiverRaidGame.h"
#include "RenderOptions.h"

// Presentation only: reads the model, submits OpenGL geometry, changes no gameplay.
class RiverRaidView
{
public:
    void Draw(const RiverRaid::RiverRaidGame& game, const RenderOptions& options = {}) const;
private:
    void DrawOverlay(const RiverRaid::RiverRaidGame& game) const;
};
