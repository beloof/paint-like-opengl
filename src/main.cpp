#include <GL/freeglut.h>

#include "canvas.h"
#include "brush.h"

// --- Canvas ---
const int canvasWidth = 800;
const int canvasHeight = 600;
int brushsize = 30;
Canvas canvas(canvasWidth, canvasHeight);
Brush brush = createCircularBrush(5);

// --- Etat de l'outil ---
unsigned char currentColor[3] = { 0, 0, 0 }; // noir par défaut


bool mouseDown = false;
int lastX = -1, lastY = -1;

// --- Utilitaires canvas ---


// --- GLUT / rendu ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glRasterPos2i(-1, -1);
    glDrawPixels(canvasWidth, canvasHeight, GL_RGB, GL_UNSIGNED_BYTE, canvas.getPixels().data());
    glutSwapBuffers();
}

// GLUT donne les coordonnées souris avec l'origine en haut-gauche,
// alors que le buffer de pixels est indexé bas-gauche (comme OpenGL) :
// on inverse donc l'axe Y ici.
int toCanvasY(int glutY) {
    return canvasHeight - 1 - glutY;
}

void mouseButton(int button, int state, int x, int y) {
    if (button != GLUT_LEFT_BUTTON) return;

    if (state == GLUT_DOWN) {
        mouseDown = true;
        lastX = x;
        lastY = toCanvasY(y);
        canvas.drawBrushLine(lastX, lastY, lastX, lastY, brushsize, currentColor, brush);
        glutPostRedisplay();
    }
    else if (state == GLUT_UP) {
        mouseDown = false;
    }
}

void mouseMotion(int x, int y) {
    if (!mouseDown) return;
    int cy = toCanvasY(y);
    canvas.drawBrushLine(lastX, lastY, x, cy, brushsize, currentColor, brush);
    lastX = x;
    lastY = cy;
    glutPostRedisplay();
}

void keyboard(unsigned char key, int, int) {
    switch (key) {
    case '1': currentColor[0] = 0;   currentColor[1] = 0;   currentColor[2] = 0;   break; // noir
    case '2': currentColor[0] = 255; currentColor[1] = 0;   currentColor[2] = 0;   break; // rouge
    case '3': currentColor[0] = 0;   currentColor[1] = 160; currentColor[2] = 0;   break; // vert
    case '4': currentColor[0] = 0;   currentColor[1] = 0;   currentColor[2] = 255; break; // bleu
    case 'e': currentColor[0] = 255; currentColor[1] = 255; currentColor[2] = 255; break; // gomme (= blanc)
    case '+': brushsize = std::min(brushsize + 1, 50); break;
    case '-': brushsize = std::max(brushsize - 1, 1);  break;
    case 'c': canvas.clearCanvas(); break;
    case 27: exit(0); break; // Echap
    }
    glutPostRedisplay();
}


int main(int argc, char** argv) {
    canvas.clearCanvas();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(canvasWidth, canvasHeight);
    glutCreateWindow("Paint-like OpenGL");

    glutDisplayFunc(display);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}