#include "game/CollisionShapes.h"
#include "game/CollisionData.h"

namespace RiverRaid
{
    bool CollisionShape::Solid(int x, int y, bool mirrored) const
    {
        if (x < 0 || x >= Width() || y < 0 || y >= Height()) return false;
        int column = x / scaleX;
        if (mirrored) column = 7 - column;
        return (rows[y] & (0x80 >> column)) != 0;
    }

    CollisionShape JetCollisionShape(bool banking)
    {
        return {banking ? std::span<const uint8_t>(CollisionData::JetMove) : std::span<const uint8_t>(CollisionData::JetStraight)};
    }

    CollisionShape ObjectCollisionShape(ObjectKind kind, int tick)
    {
        switch (kind)
        {
        case ObjectKind::Ship: return {CollisionData::Ship, 2};
        case ObjectKind::Helicopter: return {(tick / 8) % 2 ? std::span<const uint8_t>(CollisionData::Heli1) : std::span<const uint8_t>(CollisionData::Heli0)};
        case ObjectKind::Plane: return {CollisionData::Plane};
        case ObjectKind::Fuel: return {CollisionData::Fuel};
        case ObjectKind::House: return {CollisionData::House, 2};
        case ObjectKind::Bridge: return {CollisionData::Bridge, 4};
        }
        return {};
    }

}
