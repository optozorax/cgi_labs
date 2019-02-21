#define GLM_ENABLE_EXPERIMENTAL

#include <GL/freeglut.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

#include "interface.h"

//-----------------------------------------------------------------------------
void printText(int x, int y, const char *string) {
	int len, i;

	glRasterPos2f(x, y);
	len = (int) strlen(string);
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, string[i]);
	}
}


//-----------------------------------------------------------------------------
bool MovableObject::processMouse(int button, int state, vec2 pos) {
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

//-----------------------------------------------------------------------------
bool MovableObject::processMotion(vec2 pos) {
	if (m_isMoved) {
		offset(pos - m_lastPos);
		m_lastPos = pos;
		glutPostRedisplay();
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
bool MovableObject::processPassiveMotion(vec2 pos) {
	if (isPointInside(pos)) {
		m_isActive = true;
		return true;
	} else {
		m_isActive = false;
		return false;
	}
}

//-----------------------------------------------------------------------------
bool MovableObject::isActive(void) const {
	return m_isActive;
}

//-----------------------------------------------------------------------------
bool MovableObject::isMoved(void) const {
	return m_isMoved;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void PointInterface::display(void) {
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

//-----------------------------------------------------------------------------
bool PointInterface::isPointInside(vec2 point) const {
	return length(point - pos) < m_r;
}

//-----------------------------------------------------------------------------
void PointInterface::offset(vec2 point) {
	pos += point;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void QuadStripInterface::display(void) {
	// Рисуем сам объект
	glColor4ub(color.r, color.g, color.b, color.a);
	glBegin(GL_QUAD_STRIP);
	for (auto& i : points)
		glVertex2f(i.pos.x, i.pos.y);
	glEnd();

	// Если он активен, то рисуем линии и точки на его вершинах
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

	// Выводим информацию о цвете, если изменяется цвет
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

//-----------------------------------------------------------------------------
bool QuadStripInterface::isPointInside(vec2 point) const {
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

//-----------------------------------------------------------------------------
void QuadStripInterface::offset(vec2 point) {
	for (auto& i : points)
		i.pos += point;
}

//-----------------------------------------------------------------------------
bool QuadStripInterface::processMouse(int button, int state, vec2 pos) {
	// Сначала обрабатываются точки, если объект не перемещается, а лишь затем обрабатывается сам объект
	if (isActive() && !isMoved()) {
		for (auto& i : points) {
			if (i.processMouse(button, state, pos))
				return true;
		}
	}
	
	return MovableObject::processMouse(button, state, pos);
}

//-----------------------------------------------------------------------------
bool QuadStripInterface::processMotion(vec2 pos) {
	// Сначала обрабатываются точки, если объект не перемещается, а лишь затем обрабатывается сам объект
	if (isActive() && !isMoved()) {
		for (auto& i : points) {
			if (i.processMotion(pos))
				return true;
		}
	}

	return MovableObject::processMotion(pos);
}

//-----------------------------------------------------------------------------
bool QuadStripInterface::processPassiveMotion(vec2 pos) {
	// Сначала обрабатываются точки, если объект не перемещается, а лишь затем обрабатывается сам объект
	if (isActive() && !isMoved()) {
		for (auto& i : points) {
			if (i.processPassiveMotion(pos))
				return true;
		}
	}

	return MovableObject::processPassiveMotion(pos);
}

//-----------------------------------------------------------------------------
bool QuadStripInterface::processKeyboard(unsigned char key, vec2 pos) {
	if (isActive()) {
		// Здесь обрабатывается только увеличение в размерах и изменение цвета
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

//-----------------------------------------------------------------------------
bool QuadStripInterface::processWheel(int button, int dir, vec2 pos) {
	// Обработка мыши для вращения всего объекта вокруг мыши
	mat3 rotate1 = translate(rotate(translate(glm::mat3(1.0f), pos), radians(3.0f)), -pos);
	mat3 rotate2 = translate(rotate(translate(glm::mat3(1.0f), pos), radians(-3.0f)), -pos);

	if (isActive()) {
		if (dir > 0) {
			for (auto& i : points) {
				auto res = rotate1 * vec3(i.pos.x, i.pos.y, 1);
				i.pos = vec2(res.x, res.y);
			}
		} else {
			for (auto& i : points) {
				auto res = rotate2 * vec3(i.pos.x, i.pos.y, 1);
				i.pos = vec2(res.x, res.y);
			}
		}
		return true;
	} else 
		return false;
}