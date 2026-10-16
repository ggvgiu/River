#include "rendering/visuals/models/BuiltinModels.h"
#include <array>

namespace RiverRaid::Visuals
{
namespace
{
    // Visual artwork only. Collision silhouettes live in CollisionData.h.
    constexpr std::array<uint8_t, 14> Ship = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0xFC, 0xFE, 0xFF, 0x78, 0x30, 0x10, 0x10 };
    Color Palette(int row) { return row < 10 ? Color{0.46f,0.73f,0.72f} : Color{0.93f,0.53f,0.25f}; }
}

AssetModel MakeShip()
{
    AssetModel model;
    model.altitude = 0.5f;
    model.frames.push_back(MakePixelFrame(Ship, 2, 2, Palette));
    auto& frame=model.frames[0];
    frame.boxes.clear();
    // Broadside patrol boat; mirroring reverses its heading.
    frame.boxes.push_back({1,7,14,4,-0.4f,1.2f,{0.30f,0.52f,0.55f}});
    frame.boxes.push_back({0,8,1,2,0,1.1f,{0.30f,0.52f,0.55f}});
    frame.boxes.push_back({15,8,1,2,0,1.1f,{0.30f,0.52f,0.55f}});
    frame.boxes.push_back({2,7.5f,12,3,1.2f,1.7f,{0.58f,0.74f,0.71f}});
    frame.boxes.push_back({6,8,4,2,1.7f,4,{0.91f,0.65f,0.35f}});
    frame.boxes.push_back({6.5f,7.9f,3,0.2f,2.6f,3.4f,{0.12f,0.25f,0.31f}});
    frame.boxes.push_back({5,8.5f,1,1,1.7f,4.6f,{0.23f,0.26f,0.29f}});
    frame.boxes.push_back({10,8.8f,0.4f,0.4f,1.7f,5.3f,{0.72f,0.76f,0.71f}});
    return model;
}
}
