#pragma once
#include <cstdint>
#include <array>
#include <span>
#include <vector>

namespace RiverRaid::Visuals
{
    struct Color { float r, g, b; };
    struct PixelRun
    {
        float x, y, width;
        Color color;
    };
    struct ModelBox
    {
        float x, forward, width, length, bottom, top;
        Color color;
        int groundAnchor = -1;
    };
    struct AssetFrame
    {
        float width = 0;
        float length = 0;
        std::vector<PixelRun> pixels; // Independent artwork for the top-down view.
        std::vector<ModelBox> boxes; // Independent geometry for the valley view.
        struct Vertex { float x, altitude, forward; };
        struct Triangle { std::array<Vertex,3> vertices; Color color; };
        std::vector<Triangle> triangles; // Optional custom/imported mesh geometry.
        struct GroundAnchor
        {
            float x, forward, width, length;
            Color foundation;
        };
        std::vector<GroundAnchor> groundAnchors;
    };
    struct AssetModel
    {
        std::vector<AssetFrame> frames;
        int frameTicks = 8;
        float altitude = 0.5f;
        bool groundMounted = false;
        bool mirrorsWithDirection = true;
        float explosionSize = 8;
        const AssetFrame& Frame(int tick) const;
    };

    // Convenience for the initial retro models. A replacement may populate pixels
    // and boxes independently, without using a bitmap or any collision data.
    AssetFrame MakePixelFrame(std::span<const uint8_t> rows, int scaleX, float thickness,
        Color (*pixelPalette)(int), Color (*volumePalette)(int) = nullptr);

    // Turn a range of pixel-built boxes into a vertical facade. The pixel artwork
    // and collision shapes are untouched; forwardAnchor locates its base in 3D.
    void StandPixelRows(AssetFrame& frame, float firstRow, float lastRow, float forwardAnchor);
}
