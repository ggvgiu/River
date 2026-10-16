#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace RiverRaid
{
    constexpr uint16_t DefaultSeed = 0xA814;
    constexpr int FieldWidth = 160;
    constexpr int FieldHeight = 160;
    constexpr int LevelLength = 16 * 64;
    constexpr int JetBottom = 0;
    constexpr int RetainedSceneLines = 128; // Passed world still presented by the 3D camera.

    enum class ObjectKind { Ship, Helicopter, Plane, Fuel, House, Bridge };

    struct RiverRow
    {
        float bank = 44;
        float island = 0;
        bool road = false;
    };

    struct RiverObject
    {
        ObjectKind kind = ObjectKind::House;
        float x = 0;
        float distance = 0; // World coordinates increase upstream.
        int direction = 1;
        bool moving = false;
        bool destroyed = false;
        int explosionTicks = 0;
    };

    // The original NextRandom16: seed $A814, feedback = high bits 7 XOR 4.
    class CourseRandom
    {
    public:
        explicit CourseRandom(uint16_t seed = DefaultSeed) : value_(seed) {}
        void Next();
        uint8_t Low() const { return uint8_t(value_); }
        uint8_t High() const { return uint8_t(value_ >> 8); }
        uint16_t Value() const { return value_; }
    private:
        uint16_t value_;
    };

    class RiverCourse
    {
    public:
        RiverCourse(int difficulty, uint16_t seed);
        RiverRow Row(float distance) const;
        bool IsWater(float x, float distance) const;
        int Difficulty() const { return difficulty_; }
        uint16_t NextSeed() const { return nextSeed_; }
        std::vector<RiverObject>& Objects() { return objects_; }
        const std::vector<RiverObject>& Objects() const { return objects_; }
    private:
        int difficulty_;
        uint16_t nextSeed_;
        std::array<RiverRow, LevelLength> rows_{};
        std::vector<RiverObject> objects_;
    };
}
