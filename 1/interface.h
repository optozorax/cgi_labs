#pragma once

#include <vector>
#include <glm/glm.hpp>

using namespace glm;

//-----------------------------------------------------------------------------
/** Функция вывода текста на экран средствами OpenGL. */
void printText(int x, int y, const char *string);

//-----------------------------------------------------------------------------
/** Класс цвета. */
struct Color {
	Color() : r(255), g(255), b(255), a(255) {}
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}

	uint8_t r, g, b, a;
};

//-----------------------------------------------------------------------------
/**
	Класс передвигаемого объекта. Все объекты, наследуемые от него, могут быть перемещены на экране при помощи мыши.
*/
class MovableObject {
public:
	MovableObject() : m_isMoved(false), m_isActive(false), m_lastPos() {}

	// Эти функции должен реализовать наследующий класс
	virtual void display(void) = 0;
	virtual bool isPointInside(vec2 point) const = 0;
	virtual void offset(vec2 point) = 0;

	// Эти функции реализованы текущим абстрактным классом
	virtual bool processMouse(int button, int state, vec2 pos);
	virtual bool processMotion(vec2 pos);
	virtual bool processPassiveMotion(vec2 pos);

	// Проверки насчет текущего состояния передвигаемого объекта
	bool isActive(void) const;
	bool isMoved(void) const;
private:
	bool m_isMoved;
	bool m_isActive;
	vec2 m_lastPos;
};

//-----------------------------------------------------------------------------
/** Класс перемещаемой с помощью мыши точки. 
	Во время неактивного состояния рисуется серым цветом.
	Во время наведения мыши, рисуется оранжевым цветом.
	Во время передвижения рисуется синим цветом. 
*/
class PointInterface : public MovableObject {
public:
	PointInterface(vec2 pos, double r) : pos(pos), m_r(r) {}

	void display(void);
	bool isPointInside(vec2 point) const;
	void offset(vec2 point);

	vec2 pos;
private:
	double m_r;
};

//-----------------------------------------------------------------------------
/** Класс изменяемого с помощью мыши объекта Quad Strip. */
class QuadStripInterface : public MovableObject  {
public:
	QuadStripInterface() : color(0, 0, 0, 65), m_drawColorInf(false) {}

	void display(void);
	bool isPointInside(vec2 point) const;
	void offset(vec2 point);

	bool processMouse(int button, int state, vec2 pos);
	bool processMotion(vec2 pos);
	bool processPassiveMotion(vec2 pos);
	bool processKeyboard(unsigned char key, vec2 pos);
	bool processWheel(int button, int dir, vec2 pos);

	Color color;
	std::vector<PointInterface> points;
private:
	bool m_drawColorInf;
};