#pragma once
#include <algorithm>

constexpr int minBrushSize = 1;
constexpr int maxBrushSize = 150;

constexpr int paletteCount = 12;
extern const unsigned char palette[paletteCount][3];

// What the user currently has selected. Plain data: no GL, no drawing.
struct ToolState
{
    int  colorIndex = 0;
    bool eraser     = false;
    int  brushSize  = 40;
    int  brushIndex = 0;

    // Color to paint with (white when the eraser is on).
    const unsigned char* color() const;

    void setBrushSize(int s) { brushSize = std::max(minBrushSize, std::min(maxBrushSize, s)); }
    void changeBrushSize(int delta) { setBrushSize(brushSize + delta); }
};
