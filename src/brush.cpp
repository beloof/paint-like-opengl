#include "brush.h"
#include <vector>
#include <cmath>
#include <algorithm>

Brush::Brush(int width, int height)
{
	this->width = width;
	this->height = height;
	mask = std::vector<float>(width * height, 0.0f);
}

void Brush::setPixel(int x, int y, float value)
{
	if (x < 0 || x >= width || y < 0 || y >= height)
	{
		return;
	}
	mask[y * width + x] = value;
}

float Brush::getPixel(int x, int y) const
{
	if (x < 0 || x >= width || y < 0 || y >= height)
	{
		return 0.0f;
	}
	return mask[y * width + x];
}


int Brush::getWidth() const
{
	return width;
}

int Brush::getHeight() const
{
	return height;
}

void Brush::applyToCanvas(int cx, int cy, float size, const unsigned char color[3], std::vector<unsigned char>& canvasPixels, int canvasWidth, int canvasHeight) const
{
	int xmin = floor(cx - size / 2);
	int ymin = floor(cy - size / 2);
	int xmax = ceil(xmin + size);
	int ymax = ceil(ymin + size);
	for (int y = ymin; y < ymax; y++) {
		for (int x = xmin; x < xmax; x++) {
			if (x < 0 || x >= canvasWidth || y < 0 || y >= canvasHeight) {
				continue;
			}
			float xpos = (float)(x - xmin + 0.5f) * width / (float)size;
			float ypos = (float)(y - ymin + 0.5f) * height / (float)size;
			float fx = xpos - floor(xpos);
			float fy = ypos - floor(ypos);
			float opacity = 0.0f;
			float r = 0.0f, g = 0.0f, b = 0.0f;

			int index = (y * canvasWidth + x) * 3;
			r = canvasPixels[index + 0];
			g = canvasPixels[index + 1];
			b = canvasPixels[index + 2];
		
			float topLeft, topRight, bottomLeft, bottomRight;
			topLeft = mask[std::min((int)floor(xpos), width - 1) + std::min((int)floor(ypos) + 1, height - 1) * width];
			topRight = mask[std::min((int)floor(xpos) + 1, width - 1) + std::min((int)floor(ypos) + 1, height - 1) * width];
			bottomLeft = mask[std::min((int)floor(xpos), width - 1) + std::min((int)floor(ypos), height - 1) * width];
			bottomRight = mask[std::min((int)floor(xpos) + 1, width - 1) + std::min((int)floor(ypos), height - 1) * width];
			opacity += topLeft * (1.0f - fx) * fy;
			opacity += topRight * fx * fy;
			opacity += bottomLeft * (1.0f - fx) * (1.0f - fy);
			opacity += bottomRight * fx * (1.0f - fy);
			if (opacity > 0.0f) {
				int index = (y * canvasWidth + x) * 3;
				canvasPixels[index + 0] = (unsigned char)(r * (1.0f - opacity) + color[0] * opacity);
				canvasPixels[index + 1] = (unsigned char)(g * (1.0f - opacity) + color[1] * opacity);
				canvasPixels[index + 2] = (unsigned char)(b * (1.0f - opacity) + color[2] * opacity);
			}
		}
	}
}

void Brush::loadFromFile(const std::string& filename)
{

}

Brush createCircularBrush(int radius) {
	Brush brush(radius * 2 + 1, radius * 2 + 1);
	for (int i = 0; i < radius * 2 + 1; i++) {
		for (int j = 0; j < radius * 2 + 1; j++) {
			int dx = i - radius;
			int dy = j - radius;
			if (dx * dx + dy * dy <= radius * radius) {
				brush.setPixel(i, j, 1.0f);
			}
		}
	}
	return brush;
}