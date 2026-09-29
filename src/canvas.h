#pragma once

#include "brush.h"
#include <vector>

class Canvas
{
private:
    int canvasWidth;
    int canvasHeight;
    std::vector<unsigned char> pixels;

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

    void drawBrushLine(
        int x0,
        int y0,
        int x1,
        int y1,
        float size,
        const unsigned char color[3],
		const Brush& brush
    );

	void applyBrush(int cx, int cy, const unsigned char color[3], const Brush& brush, float size);

    void clearCanvas();
};