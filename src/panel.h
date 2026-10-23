#pragma once
#include "brush_library.h"
#include "gfx.h"
#include "tool_state.h"

enum class PanelAction { None, ClearCanvas };

// The side panel: draws itself and turns mouse input into ToolState changes.
// It knows nothing about the canvas; actions it can't do itself are returned.
class Panel
{
public:
    explicit Panel(const BrushLibrary& brushes) : brushes(brushes) {}

    void draw(const ToolState& tool) const;

    PanelAction press(int x, int y, ToolState& tool);  // mouse down inside the panel
    void drag(int x, ToolState& tool);                 // mouse moved while pressed
    void release() { draggingSlider = false; }
    bool isDragging() const { return draggingSlider; }

private:
    // Layout: one rect per widget, shared by drawing and hit testing.
    Rect swatchRect(int i) const;
    Rect eraserRect() const;
    Rect brushRect(int i) const;
    Rect sliderRect() const;
    Rect clearRect() const;

    void setSizeFromMouse(int x, ToolState& tool) const;

    const BrushLibrary& brushes;
    bool draggingSlider = false;
};
