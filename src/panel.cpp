#include "panel.h"

#include <GL/freeglut.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

#include "layout.h"

// ---------- Layout ----------

// Rect from a TOP-based y, so the panel can be laid out top to bottom.
static Rect topRect(int x, int top, int w, int h)
{
    return { x, layout::winH - top - h, w, h };
}

Rect Panel::swatchRect(int i) const
{
    return topRect(12 + (i % 4) * 38, 34 + (i / 4) * 38, 32, 32);
}
Rect Panel::eraserRect() const { return topRect(12, 154, 146, 26); }
Rect Panel::brushRect(int i) const
{
    return topRect(12 + (i % 3) * 50, 214 + (i / 3) * 50, 44, 44);
}
Rect Panel::sliderRect() const { return topRect(12, 396, 146, 20); }
Rect Panel::clearRect() const  { return topRect(12, 548, 146, 32); }
Rect Panel::undoRect() const   { return topRect(12, 504, 71, 32); }
Rect Panel::redoRect() const   { return topRect(87, 504, 71, 32); }

// ---------- Drawing ----------

void Panel::draw(const ToolState& tool, bool canUndo, bool canRedo) const
{
    gfx::fillRect({ 0, 0, layout::panelWidth, layout::winH }, 0.20f, 0.20f, 0.23f);

    // Colors
    glColor3f(0.85f, 0.85f, 0.85f);
    gfx::drawText(12, layout::winH - 22, "Colors");
    for (int i = 0; i < paletteCount; i++)
    {
        Rect r = swatchRect(i);
        gfx::fillRect(r, palette[i][0] / 255.0f, palette[i][1] / 255.0f, palette[i][2] / 255.0f);
        if (i == tool.colorIndex && !tool.eraser)
            gfx::outlineRect(r, 2, 1, 1, 1);
    }

    gfx::drawButton(eraserRect(), "Eraser (E)", tool.eraser);

    // Brushes
    glColor3f(0.85f, 0.85f, 0.85f);
    gfx::drawText(12, layout::winH - 206, "Brush");
    for (int i = 0; i < brushes.count(); i++)
    {
        Rect r = brushRect(i);
        gfx::fillRect(r, 1, 1, 1);
        glRasterPos2i(r.x + 2, r.y + 2);
        glDrawPixels(brushThumbSize, brushThumbSize, GL_RGB, GL_UNSIGNED_BYTE, brushes.thumb(i).data());
        if (i == tool.brushIndex)
            gfx::outlineRect(r, 2, 0.35f, 0.65f, 1.0f);
    }

    // Size slider
    char label[32];
    std::snprintf(label, sizeof(label), "Size: %d", tool.brushSize);
    glColor3f(0.85f, 0.85f, 0.85f);
    gfx::drawText(12, layout::winH - 386, label);

    Rect s = sliderRect();
    gfx::fillRect({ s.x, s.y + s.h / 2 - 3, s.w, 6 }, 0.40f, 0.40f, 0.45f);
    float t = (tool.brushSize - minBrushSize) / (float)(maxBrushSize - minBrushSize);
    int knobX = s.x + (int)(t * s.w);
    gfx::fillRect({ knobX - 5, s.y, 10, s.h }, 0.85f, 0.85f, 0.90f);

    gfx::drawButton(undoRect(), "Undo", false, canUndo);
    gfx::drawButton(redoRect(), "Redo", false, canRedo);
    gfx::drawButton(clearRect(), "Clear (C)", false);
}

// ---------- Input ----------

void Panel::setSizeFromMouse(int x, ToolState& tool) const
{
    Rect s = sliderRect();
    float t = (x - s.x) / (float)s.w;
    t = std::max(0.0f, std::min(1.0f, t));
    tool.setBrushSize(minBrushSize + (int)std::lround(t * (maxBrushSize - minBrushSize)));
}

PanelAction Panel::press(int x, int y, ToolState& tool)
{
    for (int i = 0; i < paletteCount; i++)
    {
        if (swatchRect(i).contains(x, y))
        {
            tool.colorIndex = i;
            tool.eraser = false;
            return PanelAction::None;
        }
    }

    if (eraserRect().contains(x, y))
    {
        tool.eraser = !tool.eraser;
        return PanelAction::None;
    }

    for (int i = 0; i < brushes.count(); i++)
    {
        if (brushRect(i).contains(x, y))
        {
            tool.brushIndex = i;
            return PanelAction::None;
        }
    }

    Rect s = sliderRect();
    s.y -= 6; s.h += 12; // generous hit area
    if (s.contains(x, y))
    {
        draggingSlider = true;
        setSizeFromMouse(x, tool);
        return PanelAction::None;
    }

    if (undoRect().contains(x, y)) return PanelAction::Undo;
    if (redoRect().contains(x, y)) return PanelAction::Redo;
    if (clearRect().contains(x, y)) return PanelAction::ClearCanvas;

    return PanelAction::None;
}

void Panel::drag(int x, ToolState& tool)
{
    if (draggingSlider)
        setSizeFromMouse(x, tool);
}
