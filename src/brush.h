#pragma once
#include <string>
#include <vector>

// Abstract interface: a brush knows how to apply one "dab" to a canvas.
class Brush
{
public:
    virtual ~Brush() = default;

    virtual void applyToCanvas(
        int cx, int cy,
        float size,
        const unsigned char color[3],
        std::vector<unsigned char>& canvasPixels,
        int canvasWidth, int canvasHeight) const = 0;

protected:
    // Shared helper so subclasses don't each reimplement blending.
    static void blendPixel(
        std::vector<unsigned char>& canvasPixels,
        int index,
        const unsigned char color[3],
        float opacity);
};

// Brush driven by a static grayscale opacity mask.
class MaskBrush : public Brush
{
public:
    MaskBrush(int width, int height);

    void setPixel(int x, int y, float value);
    float getPixel(int x, int y) const;
    int getWidth() const;
    int getHeight() const;
    void loadFromFile(const std::string& filename);

    void applyToCanvas(
        int cx, int cy,
        float size,
        const unsigned char color[3],
        std::vector<unsigned char>& canvasPixels,
        int canvasWidth, int canvasHeight) const override;

private:
    int width, height;
    std::vector<float> mask;
};

// Example of a non-mask brush: procedural soft round brush.
class SoftBrush : public Brush
{
public:
    explicit SoftBrush(float hardness = 0.5f) : hardness(hardness) {}

    void applyToCanvas(
        int cx, int cy,
        float size,
        const unsigned char color[3],
        std::vector<unsigned char>& canvasPixels,
        int canvasWidth, int canvasHeight) const override;

private:
    float hardness; // 0 = fully soft falloff, 1 = hard edge
};

MaskBrush createCircularBrush(int radius);