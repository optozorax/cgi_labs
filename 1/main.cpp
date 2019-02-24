#include <iostream>
#include <vector>
#include <string>
#include <GL/freeglut.h>
#include "interface.h"

std::vector<QuadStripInterface> quads;
int lastUsed = 0;
int Width = 500, Height = 500;
int lastX = 0, lastY = 0;

//-----------------------------------------------------------------------------
void Display(void) {
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	for (auto& i : quads)
		i.display();

	glutSwapBuffers();
}

//-----------------------------------------------------------------------------
void Reshape(GLint w, GLint h) {
	Width = w; Height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//-----------------------------------------------------------------------------
void Keyboard(unsigned char key, int x, int y) {
	lastX = x; lastY = y;
	y = Height - y;
	vec2 pos(x, y);
	int j = 0;
	for (auto& i : quads) {
		if (i.processKeyboard(key, pos)) { lastUsed = j; break; }
		j++;
	}

	//-------------------------------------------------------------------------
	// Добавляем или удаляем точку у последнего использованного объекта
	if (key == 'a') 
		quads[lastUsed].points.push_back(PointInterface(pos, 4));

	if (key == 'd') 
		if (quads[lastUsed].points.size() > 1) 
			quads[lastUsed].points.pop_back();

	//-------------------------------------------------------------------------
	// Добавляем новый объект, или удаляем последний использованный
	if (key == 'q') {
		QuadStripInterface q;
		q.points.push_back(PointInterface(pos, 4));
		q.points.push_back(PointInterface(pos+vec2(100, 0), 4));
		q.points.push_back(PointInterface(pos+vec2(0, 100), 4));
		q.points.push_back(PointInterface(pos+vec2(100, 100), 4));
		quads.push_back(q);
	}

	if (key == 'Q') { 
		if (quads.size() > 1) {
			quads.erase(quads.begin() + lastUsed); 
			if (lastUsed == quads.size()) lastUsed--;
		}
	}

	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void Mouse(int button, int state, int x, int y) {
	lastX = x; lastY = y;
	y = Height - y;
	vec2 pos(x, y);
	int j = 0;
	for (auto& i : quads) {
		if (i.processMouse(button, state, pos)) { lastUsed = j; break; }
		j++;
	}
	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void Motion(int x, int y) {
	lastX = x; lastY = y;
	y = Height - y;
	vec2 pos(x, y);
	int j = 0;
	for (auto& i : quads) {
		if (i.processMotion(pos)) { lastUsed = j; break; }
		j++;
	}
	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void PassiveMotion(int x, int y) {
	lastX = x; lastY = y;
	y = Height - y;
	vec2 pos(x, y);
	int j = 0;
	for (auto& i : quads) {
		if (i.processPassiveMotion(pos)) { lastUsed = j; break; }
		j++;
	}
	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void MouseWheel(int button, int dir, int x, int y) {
	lastX = x; lastY = y;
	y = Height - y;
	vec2 pos(x, y);
	int j = 0;
	for (auto& i : quads) {
		if (i.processWheel(button, dir, pos)) { lastUsed = j; break; }
		j++;
	}
	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void menu(int num) {
	switch (num) {
		case 1:  Keyboard('q', lastX, lastY);     break;
		case 2:  Keyboard('Q', lastX, lastY);     break;
		case 3:  Keyboard('a', lastX, lastY);     break;
		case 4:  Keyboard('d', lastX, lastY);     break;
		case 5:  Keyboard('r', lastX, lastY);     break;
		case 6:  Keyboard('g', lastX, lastY);     break;
		case 7:  Keyboard('b', lastX, lastY);     break;
		case 8:  Keyboard('l', lastX, lastY);     break;
		case 9:  Keyboard('R', lastX, lastY);     break;
		case 10: Keyboard('G', lastX, lastY);     break;
		case 11: Keyboard('B', lastX, lastY);     break;
		case 12: Keyboard('L', lastX, lastY);     break;
		case 13: MouseWheel(0, -1, lastX, lastY);  break;
		case 14: MouseWheel(0, 1, lastX, lastY); break;
		case 15: Keyboard('+', lastX, lastY);     break;
		case 16: Keyboard('-', lastX, lastY);     break;
		case 17: break;
	}
	glutPostRedisplay();
}

//-----------------------------------------------------------------------------
void createMenu(void) {
	int quadMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Add point             a", 3);
	glutAddMenuEntry("Delete last point     d", 4);

	glutAddMenuEntry("Increase red color    r", 5);
	glutAddMenuEntry("Increase green color  g", 6);
	glutAddMenuEntry("Increase blue color   b", 7);
	glutAddMenuEntry("Increase alpha color  l", 8);

	glutAddMenuEntry("Decrease red color    R", 9);
	glutAddMenuEntry("Decrease green color  G", 10);
	glutAddMenuEntry("Decrease blue color   B", 11);
	glutAddMenuEntry("Decrease alpha color  L", 12);

	glutAddMenuEntry("Rotate clockwise      Mouse wheel", 13);
	glutAddMenuEntry("Rotate anticlockwise  Mouse wheel", 14);

	glutAddMenuEntry("Enlarge               +", 15);
	glutAddMenuEntry("Reduce                -", 16);

	glutAddMenuEntry("Move                  Mouse", 17);

	int mainMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Add Quad Strip     q", 1);
	glutAddMenuEntry("Delete Quad Strip  Q", 2);
	glutAddSubMenu("Recently used Quad Strip menu", quadMenu);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void main(int argc, char *argv[]) {
	FreeConsole();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Quad redactor");
	createMenu();

	// Устанавливаем функции, которые будут обрабатывать события
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutPassiveMotionFunc(PassiveMotion);
	glutMouseWheelFunc(MouseWheel);

	// Включаем сглаживание
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	// Создаем первый QuadStrip
	QuadStripInterface qq;
	qq.points.push_back(PointInterface(vec2(100, 100), 4));
	qq.points.push_back(PointInterface(vec2(200, 100), 4));
	qq.points.push_back(PointInterface(vec2(100, 200), 4));
	qq.points.push_back(PointInterface(vec2(200, 200), 4));
	qq.points.push_back(PointInterface(vec2(100, 300), 4));
	qq.points.push_back(PointInterface(vec2(200, 300), 4));
	quads.push_back(qq);

	glutMainLoop();
}