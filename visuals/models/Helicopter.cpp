#include "BuiltinModels.h"
#include <array>

namespace RiverRaid::Visuals
{
namespace
{
    // Visual artwork only. Collision silhouettes live in CollisionData.h.
    constexpr std::array<uint8_t, 18> Heli0 = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x04, 0x8E, 0xFF, 0xFF, 0x9F, 0x0E, 0x04, 0x1C, 0x07, 0x00 };
    constexpr std::array<uint8_t, 18> Heli1 = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x04, 0x8E, 0xFF, 0xFF, 0x9F, 0x0E, 0x04, 0x07, 0x1C, 0x00 };
    Color Palette(int row) { return row > 13 ? Color{1,0.65f,0.22f} : Color{0.28f,0.86f,0.83f}; }
}

AssetModel MakeHelicopter()
{
    AssetModel model;
    model.altitude = 7;
    model.frames.push_back(MakePixelFrame(Heli0, 1, 2, Palette));
    model.frames.push_back(MakePixelFrame(Heli1, 1, 2, Palette));
    model.frameTicks=8; // Preserve the original 2D animation cadence.
    for(int index=0;index<2;++index)
    {
        auto& frame=model.frames[index];
        frame.boxes.clear();
        constexpr Color body{0.28f,0.86f,0.83f}, metal{0.20f,0.25f,0.27f};
        frame.boxes.push_back({4,10,3,3,0,2.2f,body});
        frame.boxes.push_back({7,10.5f,1,2,0.4f,1.8f,{0.12f,0.32f,0.42f}});
        frame.boxes.push_back({5.5f,10,1.5f,3,1.5f,2.4f,{0.17f,0.43f,0.52f}});
        frame.boxes.push_back({0.5f,11,3.5f,1,1,1.7f,body});
        frame.boxes.push_back({0,10.5f,1,2,1,3.1f,{1,0.65f,0.22f}});
        frame.boxes.push_back({3.5f,9.4f,4,0.4f,-1,-0.6f,metal});
        frame.boxes.push_back({3.5f,13.2f,4,0.4f,-1,-0.6f,metal});
        frame.boxes.push_back({4.5f,9.7f,0.4f,3.5f,-0.6f,0.3f,metal});
        frame.boxes.push_back({4.5f,11.2f,0.5f,0.5f,2.4f,3.8f,metal});
        if(index==0)
        {
            frame.boxes.push_back({0.5f,11.3f,7,0.4f,3.8f,4.0f,metal});
            frame.boxes.push_back({3.8f,8,0.4f,8,3.8f,4.0f,metal});
        }
        else for(int blade=0;blade<7;++blade)
        {
            frame.boxes.push_back({0.7f+blade,8.7f+blade*0.8f,0.8f,0.8f,3.8f,4.0f,metal});
            frame.boxes.push_back({0.7f+blade,13.5f-blade*0.8f,0.8f,0.8f,3.8f,4.0f,metal});
        }
    }
    return model;
}
}
