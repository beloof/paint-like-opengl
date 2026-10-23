#pragma once
#include "brush_library.h"
#include "canvas.h"
#include "history.h"
#include "panel.h"
#include "tool_state.h"

// Ties canvas, tools and panel together. main.cpp only forwards GLUT events here.
class App
{
public:
    App();

    void init();     // load brushes (no GL needed)
    void initGL();   // GL state + canvas texture; call after the window exists

    void display();
    void reshape(int w, int h);
    void mouseButton(int button, int state, int x, int y);
    void mouseMotion(int x, int y);
    void passiveMotion(int x, int y);
    void keyboard(unsigned char key, int modifiers);

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

    // Order matters: panel holds a reference to brushes.
    Canvas canvas;
    BrushLibrary brushes;
    ToolState tool;
    Panel panel;
    History history;

    unsigned int canvasTexture = 0; // GL texture mirroring the canvas pixels

    bool stroking = false;
    int lastX = -1, lastY = -1;   // last stroke point, canvas coordinates
    int mouseX = -1, mouseY = -1; // window coordinates, y up
};
