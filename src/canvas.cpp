#include "canvas.h"

#include <algorithm>
#include <cmath>

Canvas::Canvas(int width, int height)
{
    canvasWidth = width;
    canvasHeight = height;

    pixels = std::vector<unsigned char>(
        canvasWidth * canvasHeight * 3,
        255
    );
}

const std::vector<unsigned char>& Canvas::getPixels() const
{
    return pixels;
}

void Canvas::markDirty(int x0, int y0, int x1, int y1)
{
    x0 = std::max(x0, 0);
    y0 = std::max(y0, 0);
    x1 = std::min(x1, canvasWidth);
    y1 = std::min(y1, canvasHeight);
    if (x0 >= x1 || y0 >= y1) return;

    if (!dirty)
    {
        dirtyX0 = x0; dirtyY0 = y0; dirtyX1 = x1; dirtyY1 = y1;
        dirty = true;
    }
    else
    {
        dirtyX0 = std::min(dirtyX0, x0);
        dirtyY0 = std::min(dirtyY0, y0);
        dirtyX1 = std::max(dirtyX1, x1);
        dirtyY1 = std::max(dirtyY1, y1);
    }
}

bool Canvas::takeDirtyRect(int& x, int& y, int& w, int& h)
{
    if (!dirty) return false;
    x = dirtyX0;
    y = dirtyY0;
    w = dirtyX1 - dirtyX0;
    h = dirtyY1 - dirtyY0;
    dirty = false;
    return true;
}

void Canvas::setPixel(int x, int y, const unsigned char color[3])
{
    if (x < 0 || x >= canvasWidth ||
        y < 0 || y >= canvasHeight)
    {
        return;
    }

    int index = (y * canvasWidth + x) * 3;

    pixels[index + 0] = color[0];
    pixels[index + 1] = color[1];
    pixels[index + 2] = color[2];
    markDirty(x, y, x + 1, y + 1);
}

std::vector<unsigned char> Canvas::getPixel(int x, int y) const
{
    if (x < 0 || x >= canvasWidth ||
        y < 0 || y >= canvasHeight)
    {
        return { 255, 255, 255 };
    }
    int index = (y * canvasWidth + x) * 3;
    return {
        pixels[index + 0],
        pixels[index + 1],
        pixels[index + 2]
    };
}

void Canvas::drawFilledCircle(
    int cx,
    int cy,
    int radius,
    const unsigned char color[3])
{
    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {
            if (x * x + y * y <= radius * radius)
            {
                setPixel(cx + x, cy + y, color);
            }
        }
    }
}

void Canvas::beginStroke()
{
    distanceCarry = 0.0f;
    strokeStart = true;
}

void Canvas::dab(float x, float y, float size, const unsigned char color[3], const Brush& brush)
{
    brush.applyToCanvas(x, y, size, color, pixels, canvasWidth, canvasHeight);

    const int r = (int)std::ceil(size * 0.5f) + 2; // small margin for rounding
    const int ix = (int)std::floor(x);
    const int iy = (int)std::floor(y);
    markDirty(ix - r, iy - r, ix + r + 1, iy + r + 1);
}

void Canvas::drawBrushLine(
    float x0,
    float y0,
    float x1,
    float y1,
    float size,
    const unsigned char color[3],
    const Brush& brush,
    float spacing)
{
    const float step = std::max(1.0f, size * spacing);

    if (strokeStart)
    {
        dab(x0, y0, size, color, brush);
        strokeStart = false;
    }

    const float dx = x1 - x0;
    const float dy = y1 - y0;
    const float len = std::sqrt(dx * dx + dy * dy);

    float d = step - distanceCarry; // distance along this segment to the next dab
    while (d <= len)
    {
        const float t = d / len;
        dab(x0 + t * dx, y0 + t * dy, size, color, brush);
        d += step;
    }
    distanceCarry = len - (d - step); // distance travelled since the last dab
}

void Canvas::clearCanvas()
{
    std::fill(pixels.begin(), pixels.end(), 255);
    markDirty(0, 0, canvasWidth, canvasHeight);
}
