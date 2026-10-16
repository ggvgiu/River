#include "BuiltinModels.h"
#include <array>

namespace RiverRaid::Visuals
{
namespace
{
    // Visual artwork only. Collision silhouettes live in CollisionData.h.
    constexpr std::array<uint8_t, 24> Fuel = { 0xFE, 0xC6, 0xDE, 0xDE, 0xDE, 0xDE, 0xFE, 0xC6, 0xDE, 0xCE, 0xDE, 0xC6, 0xFE, 0xC6, 0xD6, 0xD6, 0xD6, 0xFE, 0xDE, 0xDE, 0xCE, 0xDE, 0x00, 0x7C };
    Color Palette(int row) { return (row / 6) % 2 ? Color{0.92f,0.25f,0.25f} : Color{0.91f,0.93f,0.83f}; }
}

AssetModel MakeFuelDepot()
{
    AssetModel model;
    model.altitude = 0.5f;
    model.mirrorsWithDirection = false;
    model.frames.push_back(MakePixelFrame(Fuel, 1, 3, Palette));
    auto& frame=model.frames[0];
    frame.boxes.clear();
    // Carrier conversion is visual only; the fuel footprint and rules stay fixed.
    frame.boxes.push_back({1,1,6,21,-0.4f,1.5f,{0.27f,0.35f,0.40f}});
    frame.boxes.push_back({2,0,4,1,0,1.3f,{0.27f,0.35f,0.40f}});
    frame.boxes.push_back({2,22,4,2,0,1.3f,{0.27f,0.35f,0.40f}});
    frame.boxes.push_back({0,1,8,21,1.5f,2,{0.40f,0.46f,0.47f}});
    frame.boxes.push_back({0.3f,2,0.35f,19,2,2.08f,{0.96f,0.84f,0.31f}});
    frame.boxes.push_back({7.3f,2,0.35f,19,2,2.08f,{0.96f,0.84f,0.31f}});
    for(int mark=0;mark<6;++mark)
        frame.boxes.push_back({3.6f,2+mark*3.0f,0.6f,1.5f,2,2.1f,{0.93f,0.93f,0.84f}});
    for(int wire=0;wire<3;++wire)
        frame.boxes.push_back({1.2f,3+float(wire),4.8f,0.15f,2,2.1f,{0.20f,0.24f,0.26f}});
    frame.boxes.push_back({6,12,1.7f,5,2,4.3f,{0.66f,0.70f,0.69f}});
    frame.boxes.push_back({5.9f,13,0.2f,2.5f,3.2f,3.8f,{0.10f,0.25f,0.31f}});
    frame.boxes.push_back({6.5f,13,0.4f,0.4f,4.3f,6,{0.50f,0.55f,0.55f}});
    frame.boxes.push_back({6,13,1.5f,0.25f,5.5f,5.9f,{0.50f,0.55f,0.55f}});
    frame.boxes.push_back({1,18,1.5f,1.5f,2,2.2f,{0.93f,0.25f,0.20f}});
    return model;
}
}
