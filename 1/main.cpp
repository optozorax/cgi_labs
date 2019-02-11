#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <GL/freeglut.h>
#include <spob/spob.h>

using namespace spob;

struct Color {
	Color() : r(255), g(255), b(255), a(255) {}
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}

	uint8_t r, g, b, a;
};

void printText(int x, int y, const char *string) {
	int len, i;

	glRasterPos2f(x, y);
	len = (int) strlen(string);
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, string[i]);
	}
}

class MovableObject {
public:
	MovableObject() : m_isMoved(false), m_isActive(false), m_lastPos() {}

	virtual void display(void) = 0;
	virtual bool isPointInside(vec2 point) const = 0;
	virtual void offset(vec2 point) = 0;

	virtual bool processMouse(int button, int state, vec2 pos) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && isPointInside(pos)) {
			m_isMoved = true;
			m_lastPos = pos;
			return true;
		}

		if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && m_isMoved) {
			offset(pos - m_lastPos);
			m_isMoved = false;
			glutPostRedisplay();
			return true;
		}
		return false;
	}

	virtual bool processMotion(vec2 pos) {
		if (m_isMoved) {
			offset(pos - m_lastPos);
			m_lastPos = pos;
			glutPostRedisplay();
			return true;
		}
		return false;
	}

	virtual bool processPassiveMotion(vec2 pos) {
		if (isPointInside(pos)) {
			m_isActive = true;
			return true;
		} else {
			m_isActive = false;
			return false;
		}
	}

	bool isActive(void) const {
		return m_isActive;
	}

	bool isMoved(void) const {
		return m_isMoved;
	}
private:
	bool m_isMoved;
	bool m_isActive;
	vec2 m_lastPos;
};

class PointInterface : public MovableObject {
public:
	PointInterface(vec2 pos, double r) : pos(pos), m_r(r) {}

	void display(void) {
		if (isMoved()) {
			glPointSize(2*m_r);
			glColor3ub(128, 128, 255);
		} else if (isActive()) {
			glPointSize(2*(m_r+1));
			glColor3ub(255, 128, 128);
		} else {
			glPointSize(2*m_r);
			glColor3ub(128, 128, 128);
		}
		glBegin(GL_POINTS);
		glVertex2f(pos.x, pos.y);
		glEnd();
	}

	bool isPointInside(vec2 point) const {
		return (point - pos).length() < m_r;
	}

	void offset(vec2 point) {
		pos += point;
	}

	vec2 pos;
private:
	double m_r;
};

class QuadStripInterface : public MovableObject  {
public:
	QuadStripInterface() : color(0, 0, 0, 65), m_drawColorInf(false) {}

	void display(void) {
		glColor4ub(color.r, color.g, color.b, color.a);
		glBegin(GL_QUAD_STRIP);
		for (auto& i : points)
			glVertex2f(i.pos.x, i.pos.y);
		glEnd();

		if (isActive()) {
			glColor3ub(0, 0, 0);
			for (int i = 0; i+4 <= points.size(); i += 2) {
				vec2 a = points[i].pos, b = points[i+1].pos, c = points[i+3].pos, d = points[i+2].pos;

				glBegin(GL_LINE_STRIP);
				glVertex2f(a.x, a.y);
				glVertex2f(b.x, b.y);
				glVertex2f(c.x, c.y);
				glVertex2f(d.x, d.y);
				glVertex2f(a.x, a.y);
				glEnd();
			}

			for (auto& i : points)
				i.display();
		}

		if (m_drawColorInf) {
			glColor3ub(0, 0, 0);
			std::string r = "r: " + std::to_string(int(color.r));
			std::string g = "g: " + std::to_string(int(color.g));
			std::string b = "b: " + std::to_string(int(color.b));
			std::string a = "a: " + std::to_string(int(color.a));
			int offset = 15;
			printText(5, 5+offset * 2, r.c_str());
			printText(5, 5+offset * 1, g.c_str());
			printText(5, 5+offset * 0, b.c_str());
			printText(5, 5+offset * 3, a.c_str());
			m_drawColorInf = false;
		}
	}

	bool isPointInside(vec2 point) const {
		auto isPointInsideTriangle = [] (vec2 v1, vec2 v2, vec2 v3, vec2 pt) -> bool {
			auto sign = [] (vec2 p1, vec2 p2, vec2 p3) -> float {
				return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
			};

			float d1, d2, d3;
			bool has_neg, has_pos;

			d1 = sign(pt, v1, v2);
			d2 = sign(pt, v2, v3);
			d3 = sign(pt, v3, v1);

			has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
			has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

			return !(has_neg && has_pos);
		};
		auto isPointInsideQuad = [&isPointInsideTriangle] (vec2 a, vec2 b, vec2 c, vec2 d, vec2 point) -> bool {
			return isPointInsideTriangle(a, b, c, point) || isPointInsideTriangle(a, c, d, point);
		};
		for (int i = 0; i+4 <= points.size(); i += 2) {
			if (isPointInsideQuad(points[i].pos, points[i+1].pos, points[i+3].pos, points[i+2].pos, point)) {
				return true;
			}
		}

		return false;
	}

	void offset(vec2 point) {
		for (auto& i : points)
			i.pos += point;
	}

	bool processMouse(int button, int state, vec2 pos) {
		if (isActive() && !isMoved()) {
			for (auto& i : points) {
				if (i.processMouse(button, state, pos))
					return true;
			}
		}
		
		return MovableObject::processMouse(button, state, pos);
	}

	bool processMotion(vec2 pos) {
		if (isActive() && !isMoved()) {
			for (auto& i : points) {
				if (i.processMotion(pos))
					return true;
			}
		}

		return MovableObject::processMotion(pos);
	}

	bool processPassiveMotion(vec2 pos) {
		if (isActive() && !isMoved()) {
			for (auto& i : points) {
				if (i.processPassiveMotion(pos))
					return true;
			}
		}

		return MovableObject::processPassiveMotion(pos);
	}

	bool processKeyboard(unsigned char key, vec2 pos) {
		if (isActive()) {
			if (key == '=' || key == '+') {
				for (auto& i : points) {
					i.pos -= pos;
					i.pos *= 1.2;
					i.pos += pos;
				}
			}
			if (key == '-') {
				for (auto& i : points) {
					i.pos -= pos;
					i.pos /= 1.2;
					i.pos += pos;
				}
			}

			if (key == 'r') { color.r += 5; m_drawColorInf = true; }
			if (key == 'g') { color.g += 5; m_drawColorInf = true; }
			if (key == 'b') { color.b += 5; m_drawColorInf = true; }
			if (key == 'l') { color.a += 5; m_drawColorInf = true; }

			if (key == 'R') { color.r -= 5; m_drawColorInf = true; }
			if (key == 'G') { color.g -= 5; m_drawColorInf = true; }
			if (key == 'B') { color.b -= 5; m_drawColorInf = true; }
			if (key == 'L') { color.a -= 5; m_drawColorInf = true; }
			
			return true;
		} else
			return false;
	}

	bool processWheel(int button, int dir, vec2 pos) {
		if (isActive()) {
			if (dir > 0) {
				for (auto& i : points)
					i.pos = rotate(i.pos, pos, deg2rad(3));
			} else {
				for (auto& i : points)
					i.pos = rotate(i.pos, pos, deg2rad(-3));
			}
			return true;
		} else 
			return false;
	}

	Color color;
	std::vector<PointInterface> points;
private:
	bool m_drawColorInf;
};

std::vector<QuadStripInterface> quads;
int lastUsed = 0;
int Width = 500, Height = 500;
int lastX = 0, lastY = 0;

void Display(void) {
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	for (auto& i : quads)
		i.display();

	glutSwapBuffers();
}


void Reshape(GLint w, GLint h) {
	Width = w; Height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void Keyboard(unsigned char key, int x, int y) {
	lastX = x; lastY = y;
	y = Height - y;
	vec2 pos(x, y);
	int j = 0;
	for (auto& i : quads) {
		if (i.processKeyboard(key, pos)) { lastUsed = j; break; }
		j++;
	}

	if (key == 'a') 
		quads[lastUsed].points.push_back(PointInterface(pos, 4));

	if (key == 'd') 
		if (quads[lastUsed].points.size() > 1) 
			quads[lastUsed].points.pop_back();

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

void createMenu(void) {
	int quadMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Add point    a", 3);
	glutAddMenuEntry("Delete last point    d", 4);

	glutAddMenuEntry("Increase red color    r", 5);
	glutAddMenuEntry("Increase green color    g", 6);
	glutAddMenuEntry("Increase blue color    b", 7);
	glutAddMenuEntry("Increase alpha color    l", 8);

	glutAddMenuEntry("Decrease red color    r", 9);
	glutAddMenuEntry("Decrease green color    g", 10);
	glutAddMenuEntry("Decrease blue color    b", 11);
	glutAddMenuEntry("Decrease alpha color    l", 12);

	glutAddMenuEntry("Rotate clockwise    Mouse wheel", 13);
	glutAddMenuEntry("Rotate anticlockwise    Mouse wheel", 14);

	glutAddMenuEntry("Enlarge    +", 15);
	glutAddMenuEntry("Reduce    -", 16);

	glutAddMenuEntry("Move    Mouse", 17);

	int mainMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Add Quad Strip    q", 1);
	glutAddMenuEntry("Delete Quad Strip    Q", 2);
	glutAddSubMenu("Recently used Quad Strip menu", quadMenu);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void main(int argc, char *argv[]) {
	FreeConsole();
//int CALLBACK WinMain(__in  HINSTANCE hInstance,__in  HINSTANCE hPrevInstance,__in  LPSTR lpCmdLine,__in  int nCmdShow){

	glutInit(&argc, argv);
	//glutInit(, 0);

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
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glEnable(GL_POINT_SMOOTH);
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	}

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