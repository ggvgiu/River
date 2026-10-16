#include "rendering/visuals/models/BuiltinModels.h"
#include <array>

namespace RiverRaid::Visuals
{
namespace
{
    // Visual artwork only. Collision silhouettes live in CollisionData.h.
    constexpr std::array<uint8_t, 18> Plane = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x38, 0x4F, 0xFF, 0xC6, 0x80, 0x00, 0x00 };
    Color Palette(int) { return Color{0.64f,0.8f,0.95f}; }
}

AssetModel MakeEnemyPlane()
{
    AssetModel model;
    model.altitude = 7;
    model.frames.push_back(MakePixelFrame(Plane, 1, 2, Palette));
    auto& frame=model.frames[0];
    frame.boxes.clear();
    constexpr Color body{0.64f,0.8f,0.95f};
    // Enemy jets cross the river; their nose points along +X before mirroring.
    frame.boxes.push_back({1,11.8f,6,1.8f,0,1.6f,body});
    frame.boxes.push_back({7,12.2f,1,1,0.2f,1.2f,body});
    frame.boxes.push_back({3,9,2,2.8f,0.2f,0.7f,body});
    frame.boxes.push_back({3,13.6f,2,2.4f,0.2f,0.7f,body});
    frame.boxes.push_back({5,12,1.5f,1.4f,1.6f,2.3f,{0.13f,0.32f,0.43f}});
    frame.boxes.push_back({1,11.9f,1.3f,1.6f,1.6f,3.5f,body});
    frame.boxes.push_back({1,10.2f,1.5f,1.6f,0.5f,0.9f,body});
    frame.boxes.push_back({1,13.6f,1.5f,1.6f,0.5f,0.9f,body});
    frame.boxes.push_back({0.5f,12.1f,0.5f,1.2f,0.2f,1.3f,{0.18f,0.21f,0.25f}});
    frame.boxes.push_back({0,12.4f,0.5f,0.6f,0.5f,1,{1,0.44f,0.12f}});
    return model;
}
}
