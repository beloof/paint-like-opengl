#include "gfx.h"

#include <GL/freeglut.h>
#include <cmath>

namespace gfx
{
    void fillRect(const Rect& r, float red, float green, float blue)
    {
        glColor3f(red, green, blue);
        glRecti(r.x, r.y, r.x + r.w, r.y + r.h);
    }

    void outlineRect(const Rect& r, int pad, float red, float green, float blue)
    {
        glColor3f(red, green, blue);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2i(r.x - pad, r.y - pad);
        glVertex2i(r.x + r.w + pad, r.y - pad);
        glVertex2i(r.x + r.w + pad, r.y + r.h + pad);
        glVertex2i(r.x - pad, r.y + r.h + pad);
        glEnd();
    }

    void drawCircle(float cx, float cy, float radius, float red, float green, float blue)
    {
        glColor3f(red, green, blue);
        glLineWidth(1.0f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 64; i++)
        {
            float a = i * 2.0f * 3.14159265f / 64.0f;
            glVertex2f(cx + radius * std::cos(a), cy + radius * std::sin(a));
        }
        glEnd();
    }

    void drawText(int x, int y, const char* text)
    {
        glRasterPos2i(x, y);
        for (; *text; ++text)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *text);
    }

    int textWidth(const char* text)
    {
        return glutBitmapLength(GLUT_BITMAP_HELVETICA_12, (const unsigned char*)text);
    }

    void drawButton(const Rect& r, const char* label, bool active, bool enabled)
    {
        if (!enabled)     fillRect(r, 0.24f, 0.24f, 0.27f);
        else if (active)  fillRect(r, 0.35f, 0.55f, 0.90f);
        else              fillRect(r, 0.32f, 0.32f, 0.36f);

        if (enabled) glColor3f(1, 1, 1);
        else         glColor3f(0.5f, 0.5f, 0.52f);
        drawText(r.x + (r.w - textWidth(label)) / 2, r.y + r.h / 2 - 4, label);
    }
}
