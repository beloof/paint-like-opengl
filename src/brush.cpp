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
        canvasPixels[index + c] = (unsigned char)(
            canvasPixels[index + c] * (1.0f - opacity) + color[c] * opacity);
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
    int cx, int cy,
    float size,
    const unsigned char color[3],
    std::vector<unsigned char>& canvasPixels,
    int canvasWidth, int canvasHeight) const
{
    int xmin = (int)std::floor(cx - size / 2);
    int ymin = (int)std::floor(cy - size / 2);
    int xmax = (int)std::ceil(xmin + size);
    int ymax = (int)std::ceil(ymin + size);

    for (int y = ymin; y < ymax; y++)
    {
        for (int x = xmin; x < xmax; x++)
        {
            if (x < 0 || x >= canvasWidth || y < 0 || y >= canvasHeight)
                continue;

            float xpos = (x - xmin + 0.5f) * width / size;
            float ypos = (y - ymin + 0.5f) * height / size;
            float fx = xpos - std::floor(xpos);
            float fy = ypos - std::floor(ypos);

            int x0 = std::min((int)std::floor(xpos), width - 1);
            int x1 = std::min((int)std::floor(xpos) + 1, width - 1);
            int y0 = std::min((int)std::floor(ypos), height - 1);
            int y1 = std::min((int)std::floor(ypos) + 1, height - 1);

            float opacity =
                mask[y1 * width + x0] * (1.0f - fx) * fy +
                mask[y1 * width + x1] * fx * fy +
                mask[y0 * width + x0] * (1.0f - fx) * (1.0f - fy) +
                mask[y0 * width + x1] * fx * (1.0f - fy);

            if (opacity > 0.0f)
            {
                blendPixel(canvasPixels, (y * canvasWidth + x) * 3, color, opacity);
            }
        }
    }
}

// ---------- SoftBrush ----------

void SoftBrush::applyToCanvas(
    int cx, int cy,
    float size,
    const unsigned char color[3],
    std::vector<unsigned char>& canvasPixels,
    int canvasWidth, int canvasHeight) const
{
    float radius = size / 2.0f;
    int xmin = (int)std::floor(cx - radius);
    int xmax = (int)std::ceil(cx + radius);
    int ymin = (int)std::floor(cy - radius);
    int ymax = (int)std::ceil(cy + radius);

    for (int y = ymin; y <= ymax; y++)
    {
        for (int x = xmin; x <= xmax; x++)
        {
            if (x < 0 || x >= canvasWidth || y < 0 || y >= canvasHeight)
                continue;

            float dist = std::sqrt((float)((x - cx) * (x - cx) + (y - cy) * (y - cy)));
            float t = dist / radius;
            if (t >= 1.0f) continue;

            // Full opacity inside 'hardness', linear falloff outside it.
            float opacity = (t <= hardness) ? 1.0f
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