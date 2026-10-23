#include "brush.h"
#include <algorithm>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdexcept>

// ---------- Brush ----------

void Brush::blendPixel(
    std::vector<unsigned char>& canvasPixels,
    int index,
    const unsigned char color[3],
    float opacity)
{
    for (int c = 0; c < 3; c++)
    {
        // +0.5 rounds instead of truncating, so many low-opacity dabs don't drift darker.
        canvasPixels[index + c] = (unsigned char)(
            canvasPixels[index + c] * (1.0f - opacity) + color[c] * opacity + 0.5f);
    }
}

// ---------- MaskBrush ----------

MaskBrush::MaskBrush(int width, int height)
    : width(width), height(height), mask(width* height, 0.0f)
{
}

void MaskBrush::setPixel(int x, int y, float value)
{
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    mask[y * width + x] = value;
}

float MaskBrush::getPixel(int x, int y) const
{
    if (x < 0 || x >= width || y < 0 || y >= height) return 0.0f;
    return mask[y * width + x];
}

int MaskBrush::getWidth() const { return width; }
int MaskBrush::getHeight() const { return height; }

void MaskBrush::loadFromFile(const std::string& filename)
{
    int w, h, channels;
    unsigned char* data = stbi_load(filename.c_str(), &w, &h, &channels, 1); // force 1 channel
    if (!data)
        throw std::runtime_error("Failed to load brush: " + filename);

    width = w;
    height = h;
    mask.resize(w * h);
    for (int i = 0; i < w * h; i++)
        mask[i] = data[i] / 255.0f;

    stbi_image_free(data);
}

void MaskBrush::applyToCanvas(
    float cx, float cy,
    float size,
    const unsigned char color[3],
    std::vector<unsigned char>& canvasPixels,
    int canvasWidth, int canvasHeight) const
{
    const float left = cx - size * 0.5f;
    const float top  = cy - size * 0.5f;

    // Clip the dab to the canvas once, instead of testing every pixel.
    const int xmin = std::max(0, (int)std::floor(left));
    const int ymin = std::max(0, (int)std::floor(top));
    const int xmax = std::min(canvasWidth,  (int)std::ceil(left + size));
    const int ymax = std::min(canvasHeight, (int)std::ceil(top + size));

    const float scaleX = width / size;
    const float scaleY = height / size;

    for (int y = ymin; y < ymax; y++)
    {
        const float py = y + 0.5f;
        if (py < top || py >= top + size) continue;

        // Row-dependent part of the bilinear sample, computed once per row.
        const float v = (py - top) * scaleY - 0.5f;
        int y0 = (int)std::floor(v);
        const float fy = v - y0;
        const int y1 = std::min(y0 + 1, height - 1);
        y0 = std::max(y0, 0);
        const float* row0 = &mask[y0 * width];
        const float* row1 = &mask[y1 * width];

        for (int x = xmin; x < xmax; x++)
        {
            const float px = x + 0.5f;
            if (px < left || px >= left + size) continue;

            const float u = (px - left) * scaleX - 0.5f;
            int x0 = (int)std::floor(u);
            const float fx = u - x0;
            const int x1 = std::min(x0 + 1, width - 1);
            x0 = std::max(x0, 0);

            const float a = row0[x0] * (1.0f - fx) + row0[x1] * fx;
            const float b = row1[x0] * (1.0f - fx) + row1[x1] * fx;
            const float opacity = a * (1.0f - fy) + b * fy;

            if (opacity > 0.0f)
                blendPixel(canvasPixels, (y * canvasWidth + x) * 3, color, opacity);
        }
    }
}

// ---------- SoftBrush ----------

void SoftBrush::applyToCanvas(
    float cx, float cy,
    float size,
    const unsigned char color[3],
    std::vector<unsigned char>& canvasPixels,
    int canvasWidth, int canvasHeight) const
{
    const float radius = size * 0.5f;
    const int xmin = std::max(0, (int)std::floor(cx - radius));
    const int ymin = std::max(0, (int)std::floor(cy - radius));
    const int xmax = std::min(canvasWidth,  (int)std::ceil(cx + radius));
    const int ymax = std::min(canvasHeight, (int)std::ceil(cy + radius));

    for (int y = ymin; y < ymax; y++)
    {
        const float dy = y + 0.5f - cy;
        for (int x = xmin; x < xmax; x++)
        {
            const float dx = x + 0.5f - cx;
            const float t = std::sqrt(dx * dx + dy * dy) / radius;
            if (t >= 1.0f) continue;

            // Full opacity inside 'hardness', linear falloff outside it.
            const float opacity = (t <= hardness) ? 1.0f
                : 1.0f - (t - hardness) / (1.0f - hardness + 1e-6f);

            blendPixel(canvasPixels, (y * canvasWidth + x) * 3, color, opacity);
        }
    }
}

// ---------- Factory ----------

MaskBrush createCircularBrush(int radius)
{
    int d = radius * 2 + 1;
    MaskBrush brush(d, d);
    for (int i = 0; i < d; i++)
    {
        for (int j = 0; j < d; j++)
        {
            int dx = i - radius;
            int dy = j - radius;
            if (dx * dx + dy * dy <= radius * radius)
                brush.setPixel(i, j, 1.0f);
        }
    }
    return brush;
}
