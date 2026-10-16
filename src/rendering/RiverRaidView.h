#pragma once
#include "game/RiverRaidGame.h"
#include "rendering/RenderOptions.h"
#include "rendering/visuals/AssetLibrary.h"

// Presentation only: reads the model, submits OpenGL geometry, changes no gameplay.
class RiverRaidView
{
public:
    void Draw(const RiverRaid::RiverRaidGame& game, const RenderOptions& options = {}) const;
    RiverRaid::Visuals::AssetLibrary& Assets() { return assets_; }
    const RiverRaid::Visuals::AssetLibrary& Assets() const { return assets_; }
private:
    RiverRaid::Visuals::AssetLibrary assets_;
    void DrawOverlay(const RiverRaid::RiverRaidGame& game) const;
};
