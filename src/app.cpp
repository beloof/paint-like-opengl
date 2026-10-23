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
    const int x0 = layout::panelWidth;
    const int x1 = x0 + layout::canvasWidth;
    const int y1 = layout::canvasHeight;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, canvasTexture);
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2i(x0, 0);
    glTexCoord2f(1, 0); glVertex2i(x1, 0);
    glTexCoord2f(1, 1); glVertex2i(x1, y1);
    glTexCoord2f(0, 1); glVertex2i(x0, y1);
    glEnd();
    glDisable(GL_TEXTURE_2D);
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

    glEnable(GL_SCISSOR_TEST);
    glScissor(layout::panelWidth, 0, layout::canvasWidth, layout::canvasHeight);
    const float r = tool.brushSize / 2.0f;
    const float cx = mouseX + 0.5f, cy = mouseY + 0.5f;
    gfx::drawCircle(cx, cy, r + 1.0f, 1, 1, 1);
    gfx::drawCircle(cx, cy, r, 0, 0, 0);
    glDisable(GL_SCISSOR_TEST);
}

// Pixel-coordinate projection; the window is kept at a fixed size.
void App::reshape(int w, int h)
{
    if (w != layout::winW || h != layout::winH)
    {
        glutReshapeWindow(layout::winW, layout::winH);
        return;
    }
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
    mouseY = layout::winH - 1 - glutY; // GLUT y is top-down, ours is bottom-up
    updateCursor();
}

void App::updateCursor() const
{
    glutSetCursor(mouseX >= layout::panelWidth ? GLUT_CURSOR_NONE : GLUT_CURSOR_LEFT_ARROW);
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
    if (button != GLUT_LEFT_BUTTON) return;
    trackMouse(x, y);

    if (state == GLUT_UP)
    {
        if (stroking) history.commit(canvas); // the stroke is now one undo step
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
        stroking = true;
        history.begin(canvas); // remember the canvas as it is before this stroke
        canvas.beginStroke();
        lastX = x - layout::panelWidth;
        lastY = mouseY;
        paintLine(lastX, lastY, lastX, lastY);
    }
    glutPostRedisplay();
}

void App::mouseMotion(int x, int y)
{
    trackMouse(x, y);

    if (panel.isDragging())
    {
        panel.drag(x, tool);
    }
    else if (stroking)
    {
        int cx = x - layout::panelWidth;
        paintLine(lastX, lastY, cx, mouseY);
        lastX = cx;
        lastY = mouseY;
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
    case 27:  std::exit(0); break; // Esc
    }
    glutPostRedisplay();
}
