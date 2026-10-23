#include "brush_library.h"

#include <iostream>

static std::vector<unsigned char> makeThumb(const Brush& b)
{
    const unsigned char black[3] = { 0, 0, 0 };
    std::vector<unsigned char> t(brushThumbSize * brushThumbSize * 3, 255);
    b.applyToCanvas(brushThumbSize / 2, brushThumbSize / 2, (float)(brushThumbSize - 4),
                    black, t, brushThumbSize, brushThumbSize);
    return t;
}

void BrushLibrary::add(std::unique_ptr<Brush> brush)
{
    thumbs.push_back(makeThumb(*brush));
    brushes.push_back(std::move(brush));
}

void BrushLibrary::loadDefaults(const std::string& dir)
{
    const char* names[] = { "round_hard", "round_medium", "round_soft", "square",
                            "chisel", "chalk", "splatter", "bristle" };

    for (const char* name : names)
    {
        auto brush = std::make_unique<MaskBrush>(1, 1);
        try
        {
            brush->loadFromFile(dir + "/" + name + ".png");
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << "\n";
            continue;
        }
        add(std::move(brush));
    }

    add(std::make_unique<SoftBrush>(0.3f));
}
