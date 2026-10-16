#pragma once
#include "RiverCourse.h"
#include <cstdint>
#include <span>

namespace RiverRaid
{
    struct CollisionShape
    {
        std::span<const uint8_t> rows; // bottom to top; bit 7 is the left pixel
        int scaleX = 1;
        int Width() const { return 8 * scaleX; }
        int Height() const { return int(rows.size()); }
        bool Solid(int x, int y, bool mirrored = false) const;
    };

    CollisionShape JetCollisionShape(bool banking);
    CollisionShape ObjectCollisionShape(ObjectKind kind, int tick);
    // Only gameplay uses these shapes. Rendering owns independent geometry.
}
