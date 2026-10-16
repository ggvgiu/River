#include "BuiltinModels.h"
#include <array>

namespace RiverRaid::Visuals
{
namespace
{
    // Visual artwork only. Collision silhouettes live in CollisionData.h.
    constexpr std::array<uint8_t, 13> Bridge = { 0x42, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x42 };
    Color Palette(int row) { return Color{0.64f + float(row % 2) * 0.12f,0.47f,0.23f}; }
    Color VolumePalette(int) { return {0.72f,0.49f,0.24f}; }
}

AssetModel MakeBridge()
{
    AssetModel model;
    model.altitude = 5;
    model.explosionSize = 20;
    model.frames.push_back(MakePixelFrame(Bridge, 4, 3, Palette, VolumePalette));
    // Supports are model geometry, not part of the gameplay collision mask.
    model.frames[0].boxes.push_back({0,0,2,13,-5,0,{0.48f,0.4f,0.29f}});
    model.frames[0].boxes.push_back({30,0,2,13,-5,0,{0.48f,0.4f,0.29f}});
    return model;
}
}
