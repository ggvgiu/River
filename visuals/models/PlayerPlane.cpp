#include "BuiltinModels.h"
#include <array>

namespace RiverRaid::Visuals
{
namespace
{
    constexpr std::array<uint8_t, 18> JetStraight = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x3E, 0x1C, 0x08, 0x49, 0x6B, 0x7F, 0x7F, 0x3E, 0x1C, 0x08, 0x08, 0x08 };
    constexpr std::array<uint8_t, 18> JetMove = { 0x00, 0x00, 0x00, 0x00, 0x02, 0x2E, 0x3C, 0x18, 0x08, 0x0A, 0x2E, 0x3E, 0x3E, 0x3C, 0x18, 0x08, 0x08, 0x08 };
    Color PlayerOne(int) { return {0.98f,0.87f,0.23f}; }
    Color PlayerTwo(int) { return {0.9f,0.92f,0.98f}; }
}

AssetModel MakePlayerPlane(bool banking, int player)
{
    AssetModel model;
    model.altitude = 7;
    model.frames.push_back(MakePixelFrame(banking ? std::span<const uint8_t>(JetMove) :
        std::span<const uint8_t>(JetStraight), 1, 1.4f, player == 0 ? PlayerOne : PlayerTwo));
    auto& frame = model.frames[0];
    frame.boxes.clear();
    const Color paint = player == 0 ? PlayerOne(0) : PlayerTwo(0);
    const Color underside{paint.r*0.65f,paint.g*0.65f,paint.b*0.65f};
    const Color glass{0.13f,0.36f,0.49f};

    // Longitudinal fuselage: belly, main body, shoulders and stepped nose.
    frame.boxes.push_back({3.5f,6,2,9,-0.3f,0.5f,underside});
    frame.boxes.push_back({3.5f,6,2,9,0.5f,1.8f,paint});
    frame.boxes.push_back({3,9,3,4,0.3f,1.3f,paint});
    frame.boxes.push_back({3.75f,15,1.5f,2,0.3f,1.4f,paint});
    frame.boxes.push_back({4,17,1,1,0.5f,1.1f,paint});

    // Unmirrored is a right turn: lower the right wing and lift the left.
    // Direction mirroring reverses this for a left turn.
    const float bank = banking ? -0.65f : 0;
    frame.boxes.push_back({1,10,2.5f,2,0.3f-bank,0.9f-bank,paint});
    frame.boxes.push_back({5.5f,10,2.5f,2,0.3f+bank,0.9f+bank,paint});
    frame.boxes.push_back({2,12,1.5f,1,0.4f-bank*0.6f,1.0f-bank*0.6f,paint});
    frame.boxes.push_back({5.5f,12,1.5f,1,0.4f+bank*0.6f,1.0f+bank*0.6f,paint});

    // Raised canopy, horizontal stabilizers and upright tail fin.
    frame.boxes.push_back({3.8f,12,1.4f,2.5f,1.8f,2.6f,glass});
    frame.boxes.push_back({4,14.5f,1,0.5f,1.4f,2.1f,glass});
    frame.boxes.push_back({2,6.5f,1.5f,1.5f,0.7f-bank*0.5f,1.2f-bank*0.5f,paint});
    frame.boxes.push_back({5.5f,6.5f,1.5f,1.5f,0.7f+bank*0.5f,1.2f+bank*0.5f,paint});
    frame.boxes.push_back({4,6,1,2,1.8f,3.8f,paint});
    frame.boxes.push_back({4,8,1,1,1.8f,2.8f,paint});

    // Exhaust is visible from the chase camera, rather than painted on the floor.
    frame.boxes.push_back({3.65f,5.5f,1.7f,0.5f,0.2f,1.3f,{0.16f,0.18f,0.2f}});
    frame.boxes.push_back({4,4.5f,1,1,0.45f,1.0f,{1,0.45f,0.12f}});
    return model;
}
}
