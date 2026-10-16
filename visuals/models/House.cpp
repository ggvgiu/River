#include "BuiltinModels.h"
#include <array>

namespace RiverRaid::Visuals
{
namespace
{
    // Visual artwork only. Collision silhouettes live in CollisionData.h.
    constexpr std::array<uint8_t, 22> House = { 0x00, 0x00, 0x04, 0x04, 0x0E, 0x1F, 0x1F, 0x0E, 0x0E, 0x04, 0x04, 0x04, 0x00, 0x00, 0xFE, 0xAA, 0xAA, 0xFE, 0xFE, 0x7C, 0x38, 0x00 };
    Color Palette(int row) { return row < 13 ? Color{0.3f,0.67f,0.26f} : Color{0.96f,0.88f,0.71f}; }
}

AssetModel MakeHouse()
{
    AssetModel model;
    model.altitude = 0;
    model.groundMounted = true;
    model.frames.push_back(MakePixelFrame(House, 2, 3, Palette));
    // Keep the original 2D artwork, but give the 3D scenery solid volumes.
    auto& frame = model.frames[0];
    frame.boxes.clear();

    // A taller exposed trunk keeps the narrow canopy clear of slopes and the house.
    frame.boxes.push_back({10,6,2,2,0,9,{0.43f,0.27f,0.12f}});
    frame.boxes.push_back({8,4,6,6,7,10,{0.25f,0.53f,0.18f}});
    frame.boxes.push_back({9,5,4,4,10,13,{0.30f,0.62f,0.22f}});
    frame.boxes.push_back({10,6,2,2,13,15,{0.38f,0.69f,0.26f}});

    // House: four walls and a pitched roof built from pixel-like steps.
    frame.boxes.push_back({1,14,12,7,0,7,{0.96f,0.88f,0.71f}});
    for (int step=0;step<4;++step)
    {
        const float inset=float(step)*2;
        frame.boxes.push_back({inset,13,14-inset*2,9,7+float(step),8+float(step),
            {0.72f+float(step)*0.04f,0.43f+float(step)*0.03f,0.23f}});
    }

    // Recess-colored doors and windows on front, rear and side walls.
    constexpr Color glass{0.12f,0.25f,0.31f};
    frame.boxes.push_back({2,13.85f,2,0.2f,3,5,glass});
    frame.boxes.push_back({10,13.85f,2,0.2f,3,5,glass});
    frame.boxes.push_back({6,13.8f,2,0.25f,0,5,{0.38f,0.24f,0.13f}});
    frame.boxes.push_back({2,20.95f,2,0.2f,3,5,glass});
    frame.boxes.push_back({10,20.95f,2,0.2f,3,5,glass});
    frame.boxes.push_back({0.85f,16,0.2f,2,3,5,glass});
    frame.boxes.push_back({12.95f,16,0.2f,2,3,5,glass});
    // Separate footprints let the tree and building sit on different elevations.
    frame.groundAnchors.push_back({10,6,2,2,{0.36f,0.29f,0.18f}});
    frame.groundAnchors.push_back({1,14,12,7,{0.52f,0.51f,0.44f}});
    for(size_t part=0;part<frame.boxes.size();++part)
        frame.boxes[part].groundAnchor=part<4?0:1;
    return model;
}
}
