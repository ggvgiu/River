#pragma once
#include "game/RiverRaidGame.h"
#include "rendering/RenderOptions.h"
#include "rendering/visuals/AssetLibrary.h"

class ValleyView
{
public:
    void Draw(const RiverRaid::RiverRaidGame& game, const RenderOptions& options,
        const RiverRaid::Visuals::AssetLibrary& assets) const;
};
