#include "app.h"

#include <GL/freeglut.h>
#include <cstdlib>

#include "gfx.h"
#include "layout.h"

App::App()
    : canvas(layout::canvasWidth, layout::canvasHeight), panel(brushes)
{
}

void App::init()
{
    brushes.loadDefaults("assets/brushes");
    canvas.clearCanvas();
}

void App::initGL()
{
    glClearColor(0.16f, 0.16f, 0.18f, 1.0f);

    // The canvas lives on the GPU as a texture. Redrawing it each frame is then
    // nearly free; only the part that changed is re-uploaded (see uploadCanvas).
    GLuint tex = 0;
    glGenTextures(1, &tex);
    canvasTexture = tex;

    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, layout::canvasWidth, layout::canvasHeight, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, canvas.getPixels().data());

    // Match state to the actual window GLUT created.
    winW = glutGet(GLUT_WINDOW_WIDTH);
    winH = glutGet(GLUT_WINDOW_HEIGHT);
    view = layout::fitView(winW, winH);
    viewAutoFit = true;

    glViewport(0, 0, winW, winH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, winW, 0, winH, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ---------- Rendering ----------

// Send only the rectangle of pixels that changed since the last frame.
void App::uploadCanvas()
{
    int x, y, w, h;
    if (!canvas.takeDirtyRect(x, y, w, h)) return;

    glBindTexture(GL_TEXTURE_2D, canvasTexture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, layout::canvasWidth);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h,
                    GL_RGB, GL_UNSIGNED_BYTE, canvas.getPixels().data());

    // Restore defaults so the panel's glDrawPixels thumbnails aren't affected.
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
}
void App::drawCanvas() const
{
    const float x0 = view.toWinX(0);
    const float y0 = view.toWinY(0);
    const float x1 = view.toWinX((float)layout::canvasWidth);
    const float y1 = view.toWinY((float)layout::canvasHeight);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, canvasTexture);
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(x0, y0);
    glTexCoord2f(1, 0); glVertex2f(x1, y0);
    glTexCoord2f(1, 1); glVertex2f(x1, y1);
    glTexCoord2f(0, 1); glVertex2f(x0, y1);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Outline so letterboxing is visible.
    gfx::outlineRect({ (int)x0, (int)y0, (int)(x1 - x0), (int)(y1 - y0) }, 1, 0.35f, 0.35f, 0.40f);
}

void App::display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    uploadCanvas();
    drawCanvas();
    panel.draw(tool, history.canUndo(), history.canRedo());
    drawBrushCursor();
    glutSwapBuffers();
}

// Circle showing the brush footprint, only over the canvas.
void App::drawBrushCursor() const
{
    if (mouseX < layout::panelWidth) return;

    // Scissor to the visible canvas rect.
    const int sx0 = (int)view.toWinX(0);
    const int sy0 = (int)view.toWinY(0);
    const int sx1 = (int)view.toWinX((float)layout::canvasWidth);
    const int sy1 = (int)view.toWinY((float)layout::canvasHeight);

    glEnable(GL_SCISSOR_TEST);
    glScissor(sx0, sy0, sx1 - sx0, sy1 - sy0);

    const float radius = (tool.brushSize / 2.0f) * view.zoom;
    const float cx = mouseX + 0.5f, cy = mouseY + 0.5f;
    gfx::drawCircle(cx, cy, radius + 1.0f, 1, 1, 1);
    gfx::drawCircle(cx, cy, radius, 0, 0, 0);
    glDisable(GL_SCISSOR_TEST);
}

// Pixel-coordinate projection; the window is kept at a fixed size.
void App::reshape(int w, int h)
{
    if (w <= 0 || h <= 0) return;         // minimized
    winW = w;
    winH = h;

    if (viewAutoFit)
        view = layout::fitView(winW, winH);
    else
        layout::clampView(view, winW, winH);   // keep user's zoom, just re-clamp

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ---------- Input ----------

void App::trackMouse(int glutX, int glutY)
{
    mouseX = glutX;
    mouseY = winH - 1 - glutY;   // was layout::winH
    updateCursor();
}

void App::updateCursor() const
{
    int cx, cy;
    glutSetCursor(windowToCanvas(mouseX, mouseY, cx, cy)
        ? GLUT_CURSOR_NONE : GLUT_CURSOR_LEFT_ARROW);
}

void App::paintLine(int x0, int y0, int x1, int y1)
{
    // +0.5 = center of the pixel under the mouse.
    canvas.drawBrushLine(x0 + 0.5f, y0 + 0.5f, x1 + 0.5f, y1 + 0.5f,
                         (float)tool.brushSize, tool.color(),
                         brushes.get(tool.brushIndex));
}

void App::mouseButton(int button, int state, int x, int y)
{
    trackMouse(x, y);

    if (button == GLUT_MIDDLE_BUTTON)
    {
        panning = (state == GLUT_DOWN);
        if (panning)
        {
            panStartX = mouseX;
            panStartY = mouseY;           
            panStartOffX = view.offsetX;
            panStartOffY = view.offsetY;
        }
        return;
    }

    if (button != GLUT_LEFT_BUTTON) return;

    if (state == GLUT_UP)
    {
        if (stroking) history.commit(canvas);
        stroking = false;
        panel.release();
        glutPostRedisplay();
        return;
    }

    if (x < layout::panelWidth)
    {
        switch (panel.press(x, mouseY, tool))
        {
        case PanelAction::ClearCanvas: clearCanvasWithHistory(); break;
        case PanelAction::Undo:        undo(); break;
        case PanelAction::Redo:        redo(); break;
        case PanelAction::None:        break;
        }
    }
    else
    {
        int cx, cy;
        if (!windowToCanvas(mouseX, mouseY, cx, cy)) return;

        stroking = true;
        history.begin(canvas);
        canvas.beginStroke();
        lastX = cx;
        lastY = cy;
        paintLine(lastX, lastY, lastX, lastY);
    }
    glutPostRedisplay();
}

void App::mouseMotion(int x, int y)
{
    trackMouse(x, y);

    if (panning)
    {
        view.offsetX = panStartOffX + (mouseX - panStartX);
        view.offsetY = panStartOffY + (mouseY - panStartY);
        viewAutoFit = false;
        layout::clampView(view, winW, winH);
    }
    else if (panel.isDragging())
    {
        panel.drag(x, tool);
    }
    else if (stroking)
    {
        int cx, cy;
        if (windowToCanvas(mouseX, mouseY, cx, cy))
        {
            paintLine(lastX, lastY, cx, cy);
            lastX = cx;
            lastY = cy;
        }
    }
    glutPostRedisplay();
}

void App::passiveMotion(int x, int y)
{
    trackMouse(x, y);
    glutPostRedisplay(); // keeps the brush cursor following the mouse
}

void App::undo()
{
    if (!stroking && history.undo(canvas)) glutPostRedisplay();
}

void App::redo()
{
    if (!stroking && history.redo(canvas)) glutPostRedisplay();
}

void App::clearCanvasWithHistory()
{
    history.begin(canvas);
    canvas.clearCanvas();
    history.commit(canvas);
}

void App::keyboard(unsigned char key, int modifiers)
{
    if (modifiers & GLUT_ACTIVE_CTRL)
    {
        // With Ctrl held GLUT reports control codes: Ctrl+Z = 26, Ctrl+Y = 25.
        if (key == 26 || key == 'z' || key == 'Z')
        {
            if (modifiers & GLUT_ACTIVE_SHIFT) redo(); // Ctrl+Shift+Z
            else                               undo();
        }
        else if (key == 25 || key == 'y' || key == 'Y')
        {
            redo();
        }
        glutPostRedisplay();
        return;
    }

    switch (key)
    {
    case '+': case '=': tool.changeBrushSize(+5); break;
    case '-':           tool.changeBrushSize(-5); break;
    case 'e': tool.eraser = !tool.eraser; break;
    case 'c': if (!stroking) clearCanvasWithHistory(); break;
    case 'f': view = layout::fitView(winW, winH); viewAutoFit = true; break;
    case 27:  std::exit(0); break; // Esc
    }
    glutPostRedisplay();
}

bool App::windowToCanvas(int wx, int wy, int& cx, int& cy) const
{
    if (wx < (int)view.offsetX) return false;
    if (wy < (int)view.offsetY) return false;

    const float fx = view.toCanvasX((float)wx);
    const float fy = view.toCanvasY((float)wy);
    if (fx < 0 || fy < 0) return false;
    if (fx >= layout::canvasWidth || fy >= layout::canvasHeight) return false;

    cx = (int)fx;
    cy = (int)fy;
    return true;
}

void App::mouseWheel(int /*wheel*/, int direction, int x, int y)
{
    if (x < layout::panelWidth) return;

    // Anchor the canvas point under the cursor while zooming.
    const float cx = view.toCanvasX((float)x);
    const float cy = view.toCanvasY((float)(winH - 1 - y));

    const float factor = (direction > 0) ? 1.15f : (1.0f / 1.15f);
    view.zoom *= factor;
    view.offsetX = x - cx * view.zoom;
    view.offsetY = (winH - 1 - y) - cy * view.zoom;
    viewAutoFit = false;

    layout::clampView(view, winW, winH);
    glutPostRedisplay();
}
