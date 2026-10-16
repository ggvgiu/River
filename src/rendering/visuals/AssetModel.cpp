#include "rendering/visuals/AssetModel.h"
#include <algorithm>

namespace RiverRaid::Visuals
{
    void StandPixelRows(AssetFrame& frame, float firstRow, float lastRow, float forwardAnchor)
    {
        for (auto& box : frame.boxes)
        {
            if (box.forward < firstRow || box.forward >= lastRow) continue;
            const float height = box.length;
            const float thickness = box.top - box.bottom;
            const float bottom = box.forward - firstRow;
            box.forward = forwardAnchor - thickness / 2;
            box.length = thickness;
            box.bottom = bottom;
            box.top = bottom + height;
        }
    }

    const AssetFrame& AssetModel::Frame(int tick) const
    {
        static const AssetFrame empty;
        if (frames.empty()) return empty;
        return frames[(std::max(0, tick) / std::max(1, frameTicks)) % frames.size()];
    }

    AssetFrame MakePixelFrame(std::span<const uint8_t> rows, int scaleX, float thickness,
        Color (*pixelPalette)(int), Color (*volumePalette)(int))
    {
        AssetFrame result;
        result.width = float(8 * scaleX);
        result.length = float(rows.size());
        for (int row = 0; row < int(rows.size()); ++row)
        for (int column = 0; column < 8;)
        {
            if (!(rows[row] & (0x80 >> column))) { ++column; continue; }
            const int start = column++;
            while (column < 8 && (rows[row] & (0x80 >> column))) ++column;
            const float x = float(start * scaleX), width = float((column - start) * scaleX);
            result.pixels.push_back({x, float(row), width, pixelPalette(row)});
            result.boxes.push_back({x, float(row), width, 1, 0, thickness,
                volumePalette ? volumePalette(row) : pixelPalette(row)});
        }
        return result;
    }
}
