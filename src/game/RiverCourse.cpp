#include "game/RiverCourse.h"
#include <algorithm>
#include <cmath>

namespace RiverRaid
{
    void CourseRandom::Next()
    {
        const unsigned feedback = ((value_ >> 15) ^ (value_ >> 12)) & 1;
        value_ = uint16_t((value_ << 1) | feedback);
    }

    RiverCourse::RiverCourse(int difficulty, uint16_t seed) : difficulty_(difficulty), nextSeed_(seed)
    {
        CourseRandom random(seed);
        int previousWidth = 12;
        int bankState = 0; // 0=banks, 1=enter island, 3=island, 2=leave island.
        bool previousIsland = false;
        const int padding = difficulty < 5 ? 6 : 0;
        constexpr std::array enemyKinds = { ObjectKind::Ship, ObjectKind::Helicopter,
            ObjectKind::Ship, ObjectKind::Helicopter, ObjectKind::Plane, ObjectKind::Ship,
            ObjectKind::Helicopter, ObjectKind::Helicopter };
        constexpr int housePositions[] = {143,141,7,10,132,13,128,18,124,22,120,26,116,30,112};

        for (int block = 0; block < 16; ++block)
        {
            random.Next();
            int width = 7;
            if (block == 15) { width = 12; bankState = 0; }
            else if ((difficulty & 1) == 0)
            {
                if (block == 14) bankState = bankState == 3 ? 2 : 0;
                else if (bankState == 1) bankState = 3;
                else if (bankState == 2) bankState = 0;
                else if ((random.Low() & 0x30) == 0) bankState = bankState == 3 ? 2 : 1;
                if (bankState == 1 || bankState == 2) width = 0;
                else
                {
                    width = random.Low() & 15;
                    if (width < 2) width += 2;
                    width = std::min(width, padding ? 8 : (bankState == 3 ? 13 : 14));
                }
            }
            const bool island = bankState == 3;
            const float targetBank = island ? 16.0f : 16.0f + width * 4.0f;
            const float targetIsland = island ? width * 4.0f : 0;
            const float oldBank = previousIsland ? 16.0f : 16.0f + previousWidth * 4.0f;
            const float oldIsland = previousIsland ? previousWidth * 4.0f : 0;

            // TIA PF cells were four pixels wide and each slope step occupied two
            // scanlines. Interpolate those steps for clean outlines at any size.
            const float slopeLength = std::max(2.0f, float(std::abs(width - previousWidth) * 2));
            const float slopeStart = width < previousWidth ? 32 - slopeLength : 0;
            for (int line = 0; line < 64; ++line)
            {
                float t = std::clamp((line - slopeStart) / slopeLength, 0.0f, 1.0f);
                rows_[block * 64 + line] = {oldBank + (targetBank - oldBank) * t,
                    oldIsland + (targetIsland - oldIsland) * t, block == 15 && line >= 42 && line < 55};
            }

            for (int zone = 0; zone < 2; ++zone)
            {
                if (zone) random.Next();
                RiverObject object;
                object.distance = float(block * 64 + zone * 32 + 4);
                object.direction = (random.Low() & 0x80) ? -1 : 1;
                if (block == 15 && zone == 1)
                {
                    object.kind = ObjectKind::Bridge;
                    object.x = 64;
                    object.distance = float(block * 64 + 42);
                }
                else
                {
                    // Native equivalents of .newEnemy / .newFuel / .newHouse.
                    if (block == 0 && zone == 0) object.kind = ObjectKind::House;
                    else if (random.High() > (63 - difficulty) * 2)
                        object.kind = enemyKinds[random.High() & (difficulty < 3 ? 1 : 7)];
                    else object.kind = (random.Low() & 0x40) ? ObjectKind::House : ObjectKind::Fuel;

                    if (object.kind == ObjectKind::House)
                    {
                        if (difficulty & 1)
                        {
                            int x = (random.Low() & 31) + 9;
                            object.x = float(x < 25 ? x : x + 93);
                        }
                        else object.x = island ? 71.0f : float(housePositions[std::min(width, previousWidth)]);
                    }
                    else if ((difficulty & 1) && width == previousWidth)
                    {
                        const int maximum = (object.kind == ObjectKind::Ship ? 97 : 106) - padding;
                        object.x = float(std::min((random.Low() & 63) + 46 + padding, maximum));
                    }
                    else
                    {
                        int widest = std::max(width, previousWidth);
                        if (widest >= 10 && object.kind == ObjectKind::Ship) object.kind = ObjectKind::Helicopter;
                        const auto row = rows_[block * 64 + zone * 32 + 16];
                        const float spriteWidth = object.kind == ObjectKind::Ship ? 16.0f : 8.0f;
                        float left = row.bank + 2 + padding;
                        float right = 160 - row.bank - spriteWidth - 2 - padding;
                        if (row.island > 0)
                        {
                            if (random.Low() & 0x80) left = 80 + row.island + 2;
                            else right = 80 - row.island - spriteWidth - 2;
                        }
                        object.x = (random.Low() & 0x80) ? right : left;
                    }
                }
                objects_.push_back(object);
            }
            previousWidth = width;
            previousIsland = island;
        }
        nextSeed_ = random.Value();
    }

    RiverRow RiverCourse::Row(float distance) const
    {
        if (distance < 0) return {64, 0, false}; // narrow approach to the first river
        const int index = std::clamp(int(distance), 0, LevelLength - 1);
        return rows_[index];
    }

    bool RiverCourse::IsWater(float x, float distance) const
    {
        const auto row = Row(distance);
        return x >= row.bank && x < FieldWidth - row.bank &&
            (row.island <= 0 || x < 80 - row.island || x >= 80 + row.island);
    }
}
