#pragma once
#include "AssetModel.h"

namespace RiverRaid::Visuals
{
    void Paint(Color color, bool monochrome, float shade = 1);
    void Box(float x, float forward, float width, float length, float bottom, float top, Color color, bool monochrome);
    void DrawPixels(const AssetFrame& frame, float x, float y, bool mirrored, bool monochrome);
    void DrawPixelShadow(const AssetFrame& frame, float x, float y, bool mirrored, bool monochrome, Color color);
    void DrawModel(const AssetFrame& frame, float x, float y, float altitude, bool mirrored, bool monochrome);
    void DrawModelShadow(const AssetFrame& frame, float x, float y, float altitude, bool mirrored, bool monochrome);
}
