#pragma once
#include "rendering/visuals/AssetModel.h"
#include "game/RiverCourse.h"
#include <array>

namespace RiverRaid::Visuals
{
    // Renderer-owned catalog. Replacing a visual asset never touches the model
    // used for collisions, scores, enemy movement, or procedural generation.
    class AssetLibrary
    {
    public:
        AssetLibrary();
        const AssetModel& Object(ObjectKind kind) const;
        const AssetModel& Player(bool banking, int player) const;
        void SetObject(ObjectKind kind, AssetModel model);
        void SetPlayer(bool banking, int player, AssetModel model);
    private:
        std::array<AssetModel, 6> objects_;
        std::array<std::array<AssetModel, 2>, 2> players_;
    };
}
