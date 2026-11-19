#pragma once
#include "brush_library.h"
#include "gfx.h"
#include "layout.h"
#include "tool_state.h"

enum class PanelAction { None, ClearCanvas, Undo, Redo, Open, Save };

// The side panel: draws itself and turns mouse input into ToolState changes.
// It knows nothing about the canvas; actions it can't do itself are returned.
class Panel
{
public:
    explicit Panel(const BrushLibrary& brushes) : brushes(brushes) {}

    void draw(const ToolState& tool, bool canUndo, bool canRedo) const;

    PanelAction press(int x, int y, ToolState& tool);  // mouse down inside the panel
    void drag(int x, ToolState& tool);                 // mouse moved while pressed
    void release() { draggingSlider = false; }
    bool isDragging() const { return draggingSlider; }

    // The panel spans the full window height. Widgets are laid out top-down in a
    // fixed-height content area (layout::canvasHeight); when the window is
    // shorter than that, the content scrolls on its own (mouse wheel over the
    // panel), independent of the canvas pan.
    void setViewHeight(int h);
    void scrollBy(int dy);    // dy > 0 moves the content up (shows lower widgets)

private:
    int maxScroll() const;
    int topY(int top) const { return viewHeight - top + scroll; } // top-based -> GL y
    Rect topRect(int x, int top, int w, int h) const { return { x, topY(top) - h, w, h }; }

    // Layout: one rect per widget, shared by drawing and hit testing.
    Rect swatchRect(int i) const;
    Rect eraserRect() const;
    Rect brushRect(int i) const;
    Rect sliderRect() const;
    Rect clearRect() const;
    Rect undoRect() const;
    Rect redoRect() const;
    Rect openRect() const;
    Rect saveRect() const;

    void setSizeFromMouse(int x, ToolState& tool) const;

    const BrushLibrary& brushes;
    bool draggingSlider = false;
    int viewHeight = layout::canvasHeight;   // current window height in pixels
    int scroll = 0;       // 0 .. maxScroll()
};
