#include "panel.h"

#include <GL/freeglut.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

#include "layout.h"

// ---------- Layout ----------

// Widgets use TOP-based y (see topRect in panel.h), measured from the top of
// the window and shifted by the scroll offset. The content is canvasHeight tall.

void Panel::setViewHeight(int h)
{
    viewHeight = h;
    scrollBy(0); // re-clamp
}

int Panel::maxScroll() const
{
    return std::max(0, layout::canvasHeight - viewHeight);
}

void Panel::scrollBy(int dy)
{
    scroll = std::max(0, std::min(maxScroll(), scroll + dy));
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
Rect Panel::openRect() const   { return topRect(12, 446, 71, 32); }
Rect Panel::saveRect() const   { return topRect(87, 446, 71, 32); }

// ---------- Drawing ----------

void Panel::draw(const ToolState& tool, bool canUndo, bool canRedo) const
{
    // Background covers the whole sidebar column, whatever the window height.
    gfx::fillRect({ 0, 0, layout::panelWidth, viewHeight }, 0.20f, 0.20f, 0.23f);

    // Colors
    glColor3f(0.85f, 0.85f, 0.85f);
    gfx::drawText(12, topY(22), "Colors");
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
    gfx::drawText(12, topY(206), "Brush");
    for (int i = 0; i < brushes.count(); i++)
    {
        Rect r = brushRect(i);
        gfx::fillRect(r, 1, 1, 1);
        // Set the raster position via a valid point plus a glBitmap offset, so
        // thumbnails partly scrolled off-window are clipped, not dropped entirely
        // (glRasterPos outside the viewport would invalidate the whole image).
        glRasterPos2i(0, 0);
        glBitmap(0, 0, 0, 0, (float)(r.x + 2), (float)(r.y + 2), nullptr);
        glDrawPixels(brushThumbSize, brushThumbSize, GL_RGB, GL_UNSIGNED_BYTE, brushes.thumb(i).data());
        if (i == tool.brushIndex)
            gfx::outlineRect(r, 2, 0.35f, 0.65f, 1.0f);
    }

    // Size slider
    char label[32];
    std::snprintf(label, sizeof(label), "Size: %d", tool.brushSize);
    glColor3f(0.85f, 0.85f, 0.85f);
    gfx::drawText(12, topY(386), label);

    Rect s = sliderRect();
    gfx::fillRect({ s.x, s.y + s.h / 2 - 3, s.w, 6 }, 0.40f, 0.40f, 0.45f);
    float t = (tool.brushSize - minBrushSize) / (float)(maxBrushSize - minBrushSize);
    int knobX = s.x + (int)(t * s.w);
    gfx::fillRect({ knobX - 5, s.y, 10, s.h }, 0.85f, 0.85f, 0.90f);

    gfx::drawButton(openRect(), "Open", false);
    gfx::drawButton(saveRect(), "Save", false);
    gfx::drawButton(undoRect(), "Undo", false, canUndo);
    gfx::drawButton(redoRect(), "Redo", false, canRedo);
    gfx::drawButton(clearRect(), "Clear (C)", false);

    // Scroll indicator on the panel's right edge, only when content overflows.
    const int range = maxScroll();
    if (range > 0)
    {
        const int trackH = viewHeight;
        const int thumbH = std::max(20, trackH * viewHeight / layout::canvasHeight);
        const int thumbTop = (trackH - thumbH) * scroll / range;
        gfx::fillRect({ layout::panelWidth - 5, viewHeight - thumbTop - thumbH, 3, thumbH },
                      0.55f, 0.55f, 0.60f);
    }
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

    if (openRect().contains(x, y)) return PanelAction::Open;
    if (saveRect().contains(x, y)) return PanelAction::Save;
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
