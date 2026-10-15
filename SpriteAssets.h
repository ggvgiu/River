#pragma once
#include "RiverCourse.h"
#include <cstdint>
#include <span>

namespace RiverRaid
{
    struct Sprite
    {
        std::span<const uint8_t> rows; // bottom to top; bit 7 is the left pixel
        int scaleX = 1;
        int Width() const { return 8 * scaleX; }
        int Height() const { return int(rows.size()); }
        bool Solid(int x, int y, bool mirrored = false) const;
    };

    Sprite JetSprite(bool banking);
    Sprite ObjectSprite(ObjectKind kind, int tick);
    int ObjectScore(ObjectKind kind);
}
