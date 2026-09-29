#pragma once

#include <vector>
#include <string>

class Brush
{
private:
    int width;
    int height;

    std::vector<float> mask;

public:
    Brush(int width, int height);

    void setPixel(int x, int y, float value);
    float getPixel(int x, int y) const;
    int getWidth() const;
    int getHeight() const;
	void applyToCanvas(int cx, int cy, float size, const unsigned char color[3], std::vector<unsigned char>& canvasPixels, int canvasWidth, int canvasHeight) const;
	void loadFromFile(const std::string& filename);
};

Brush createCircularBrush(int radius);