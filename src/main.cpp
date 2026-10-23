#include <GL/freeglut.h>

#include "app.h"
#include "layout.h"

// GLUT callbacks are plain functions, so the app lives in one global
// and these thin wrappers forward to it.
static App app;

static void display()                              { app.display(); }
static void reshape(int w, int h)                  { app.reshape(w, h); }
static void mouseButton(int b, int s, int x, int y){ app.mouseButton(b, s, x, y); }
static void mouseMotion(int x, int y)              { app.mouseMotion(x, y); }
static void passiveMotion(int x, int y)            { app.passiveMotion(x, y); }
static void keyboard(unsigned char k, int, int)    { app.keyboard(k); }

int main(int argc, char** argv)
{
    app.init();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(layout::winW, layout::winH);
    glutCreateWindow("Paint-like OpenGL");
    app.initGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutPassiveMotionFunc(passiveMotion);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}
