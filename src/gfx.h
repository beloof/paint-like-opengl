#pragma once

// Small OpenGL drawing helpers (immediate mode, pixel coordinates, origin bottom-left).

struct Rect
{
    int x, y, w, h;
    bool contains(int px, int py) const
    {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};

namespace gfx
{
    void fillRect(const Rect& r, float red, float green, float blue);
    void outlineRect(const Rect& r, int pad, float red, float green, float blue);
    void drawCircle(float cx, float cy, float radius, float red, float green, float blue);
    void drawText(int x, int y, const char* text);
    int  textWidth(const char* text);
    void drawButton(const Rect& r, const char* label, bool active, bool enabled = true);
}
