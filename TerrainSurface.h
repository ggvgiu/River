#pragma once
#include "RiverRaidGame.h"
#include "visuals/AssetModel.h"

// Rendering-only terrain. The original river rows and collision checks are intact.
namespace TerrainSurface
{
    float Height(const RiverRaid::RiverRaidGame& game, float x, float worldY);
    float FlightAltitude(const RiverRaid::RiverRaidGame& game,
        const RiverRaid::Visuals::AssetFrame& frame,float x,float worldY,float cruiseAltitude,bool mirrored);
    void Draw(const RiverRaid::RiverRaidGame& game, bool monochrome, bool seedColors = true);
    RiverRaid::Visuals::AssetFrame GroundFrame(const RiverRaid::RiverRaidGame& game,
        const RiverRaid::Visuals::AssetFrame& frame, float x, float worldY, bool mirrored);
}
