#pragma once

// Window = [ side panel | canvas view ]
// The panel is fixed at 170x600 (glued to the canvas's vertical extent).
// The canvas buffer is fixed at 800x600; the *view* of it (zoom + pan + fit)
// adapts to the window. Zoom/pan live in View so they don't touch the buffer.

namespace layout
{
    constexpr int canvasWidth = 800;
    constexpr int canvasHeight = 600;
    constexpr int panelWidth = 170;

    // Initial window size (user can resize freely after).
    constexpr int winW = panelWidth + canvasWidth;
    constexpr int winH = canvasHeight;

    // Minimum window size so the panel stays usable.
    constexpr int minWinW = panelWidth + 200;
    constexpr int minWinH = 200;

    constexpr float minZoom = 0.1f;
    constexpr float maxZoom = 16.0f;

    // Canvas-pixel -> window-pixel transform:
    //   windowX = offsetX + canvasX * zoom
    //   windowY = offsetY + canvasY * zoom
    struct View
    {
        float zoom = 1.0f;
        float offsetX = (float)panelWidth;
        float offsetY = 0.0f;

        float toWinX(float cx)    const { return offsetX + cx * zoom; }
        float toWinY(float cy)    const { return offsetY + cy * zoom; }
        float toCanvasX(float wx) const { return (wx - offsetX) / zoom; }
        float toCanvasY(float wy) const { return (wy - offsetY) / zoom; }
    };

    // A view that fits the whole canvas into the window's drawable area,
    // letterboxed, preserving the canvas's aspect ratio.
    inline View fitView(int winWNow, int winHNow)
    {
        View v;
        const int availW = winWNow - panelWidth;
        const int availH = winHNow;
        if (availW <= 0 || availH <= 0) return v;

        const float sx = (float)availW / canvasWidth;
        const float sy = (float)availH / canvasHeight;
        v.zoom = sx < sy ? sx : sy;
        v.offsetX = panelWidth + (availW - canvasWidth * v.zoom) * 0.5f;
        v.offsetY = (availH - canvasHeight * v.zoom) * 0.5f;
        return v;
    }

    // Clamp a view's zoom and keep it from scrolling the canvas entirely off.
    // Called after wheel/pan edits so the view stays sane.
    inline void clampView(View& v, int winWNow, int winHNow)
    {
        if (v.zoom < minZoom) v.zoom = minZoom;
        if (v.zoom > maxZoom) v.zoom = maxZoom;

        const float cw = canvasWidth * v.zoom;
        const float ch = canvasHeight * v.zoom;
        const float availW = (float)(winWNow - panelWidth);
        const float availH = (float)winHNow;

        // Keep at least a sliver of the canvas visible in each axis.
        if (cw <= availW)
            v.offsetX = panelWidth + (availW - cw) * 0.5f;   // centre when it fits
        else
        {
            const float minOff = panelWidth + availW - cw;
            const float maxOff = (float)panelWidth;
            if (v.offsetX < minOff) v.offsetX = minOff;
            if (v.offsetX > maxOff) v.offsetX = maxOff;
        }

        if (ch <= availH)
            v.offsetY = (availH - ch) * 0.5f;
        else
        {
            const float minOff = availH - ch;
            const float maxOff = 0.0f;
            if (v.offsetY < minOff) v.offsetY = minOff;
            if (v.offsetY > maxOff) v.offsetY = maxOff;
        }
    }
}