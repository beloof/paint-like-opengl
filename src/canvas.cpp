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

void Canvas::drawBrushLine(
    int x0,
    int y0,
    int x1,
    int y1,
    float size,
    const unsigned char color[3],
    const Brush& brush)
{
    int dx = x1 - x0;
    int dy = y1 - y0;

    int steps = std::max(std::abs(dx), std::abs(dy));

    if (steps == 0)
    {
		brush.applyToCanvas(x0, y0, size, color, pixels, canvasWidth, canvasHeight);
        return;
    }

    for (int i = 0; i <= steps; i++)
    {
		float t = (float)i / (float)steps;
        brush.applyToCanvas(
            x0 + t * dx,
            y0 + t * dy,
            size,
            color,
            pixels,
            canvasWidth,
            canvasHeight
        );
    }
    return;
}


void Canvas::clearCanvas()
{
    std::fill(pixels.begin(), pixels.end(), 255);
}