#include <GL/freeglut.h>

int windowWidth = 800;
int windowHeight = 600;

void display() {
    glClearColor(0.85f, 0.85f, 0.85f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Petit test : un triangle au centre pour vérifier que le rendu fonctionne
    glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.2f, 0.2f);
        glVertex2f(-0.5f, -0.5f);
        glColor3f(0.2f, 1.0f, 0.2f);
        glVertex2f(0.5f, -0.5f);
        glColor3f(0.2f, 0.2f, 1.0f);
        glVertex2f(0.0f, 0.5f);
    glEnd();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Paint-like OpenGL - test");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}
