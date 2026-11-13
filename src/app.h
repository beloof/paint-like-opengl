#pragma once
#include "brush_library.h"
#include "canvas.h"
#include "history.h"
#include "layout.h"
#include "panel.h"
#include "tool_state.h"

class App
{
public:
    App();

    void init();
    void initGL();

    void display();
    void reshape(int w, int h);
    void mouseButton(int button, int state, int x, int y);
    void mouseMotion(int x, int y);
    void passiveMotion(int x, int y);
    void keyboard(unsigned char key, int modifiers);
    void mouseWheel(int wheel, int direction, int x, int y);  // new, for zoom

private:
    void trackMouse(int glutX, int glutY);
    void updateCursor() const;
    void paintLine(int x0, int y0, int x1, int y1);
    void undo();
    void redo();
    void clearCanvasWithHistory();
    void uploadCanvas();
    void drawCanvas() const;
    void drawBrushCursor() const;

    // Map window point -> canvas pixel; returns false if outside the canvas.
    bool windowToCanvas(int wx, int wy, int& cx, int& cy) const;

    Canvas canvas;
    BrushLibrary brushes;
    ToolState tool;
    Panel panel;
    History history;

    unsigned int canvasTexture = 0;

    bool stroking = false;
    int lastX = -1, lastY = -1;
    int mouseX = -1, mouseY = -1;

    int winW = layout::winW;
    int winH = layout::winH;
    layout::View view;            // canvas->window transform (zoom + pan)
    bool viewAutoFit = true;      // true until the user zooms/pans; then reshape won't re-fit
    bool panning = false;         // middle-button pan in progress
    int panStartX = 0, panStartY = 0;
    float panStartOffX = 0, panStartOffY = 0;
};