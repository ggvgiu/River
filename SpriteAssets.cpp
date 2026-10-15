#include "SpriteAssets.h"
#include "SpriteData.h"

namespace RiverRaid
{
    bool Sprite::Solid(int x, int y, bool mirrored) const
    {
        if (x < 0 || x >= Width() || y < 0 || y >= Height()) return false;
        int column = x / scaleX;
        if (mirrored) column = 7 - column;
        return (rows[y] & (0x80 >> column)) != 0;
    }

    Sprite JetSprite(bool banking)
    {
        return {banking ? std::span<const uint8_t>(Assets::JetMove) : std::span<const uint8_t>(Assets::JetStraight)};
    }

    Sprite ObjectSprite(ObjectKind kind, int tick)
    {
        switch (kind)
        {
        case ObjectKind::Ship: return {Assets::Ship, 2};
        case ObjectKind::Helicopter: return {(tick / 8) % 2 ? std::span<const uint8_t>(Assets::Heli1) : std::span<const uint8_t>(Assets::Heli0)};
        case ObjectKind::Plane: return {Assets::Plane};
        case ObjectKind::Fuel: return {Assets::Fuel};
        case ObjectKind::House: return {Assets::House, 2};
        case ObjectKind::Bridge: return {Assets::Bridge, 4};
        }
        return {};
    }

    int ObjectScore(ObjectKind kind)
    {
        switch (kind)
        {
        case ObjectKind::Ship: return 30;
        case ObjectKind::Helicopter: return 60;
        case ObjectKind::Plane: return 100;
        case ObjectKind::Fuel: return 80;
        case ObjectKind::Bridge: return 500;
        default: return 0;
        }
    }
}
