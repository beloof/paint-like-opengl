#pragma once

#include "brush.h"
#include <vector>

class Canvas
{
private:
    int canvasWidth;
    int canvasHeight;
    std::vector<unsigned char> pixels;

    // Stroke state: leftover distance since the last dab, so spacing stays
    // even across the many short segments a mouse drag produces.
    float distanceCarry = 0.0f;
    bool strokeStart = true;

    // Bounding box of everything changed since the last takeDirtyRect().
    bool dirty = false;
    int dirtyX0 = 0, dirtyY0 = 0, dirtyX1 = 0, dirtyY1 = 0; // x1/y1 exclusive

    void markDirty(int x0, int y0, int x1, int y1);
    void dab(float x, float y, float size, const unsigned char color[3], const Brush& brush);

public:
    Canvas(int width = 800, int height = 600);

    const std::vector<unsigned char>& getPixels() const;

    void setPixel(int x, int y, const unsigned char color[3]);

    std::vector<unsigned char> getPixel(int x, int y) const;

    void drawFilledCircle(
        int cx,
        int cy,
        int radius,
        const unsigned char color[3]
    );

    // Call on mouse-down so the next stroke starts fresh.
    void beginStroke();

    // spacing = distance between dabs as a fraction of brush size.
    void drawBrushLine(
        float x0,
        float y0,
        float x1,
        float y1,
        float size,
        const unsigned char color[3],
        const Brush& brush,
        float spacing = 0.15f
    );

    void clearCanvas();

    // Returns false if nothing changed. Otherwise fills in the changed
    // rectangle (so the renderer only re-uploads that part) and resets it.
    bool takeDirtyRect(int& x, int& y, int& w, int& h);
};
