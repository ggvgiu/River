#pragma once
#include "RiverRaidGame.h"
#include "RenderOptions.h"
#include "visuals/AssetLibrary.h"

class ValleyView
{
public:
    void Draw(const RiverRaid::RiverRaidGame& game, const RenderOptions& options,
        const RiverRaid::Visuals::AssetLibrary& assets) const;
};
