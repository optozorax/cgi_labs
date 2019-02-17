#include <GL/glew.h>

#include "plane.h"

std::vector<Plane> ClipPlane::p_stack;

void Plane::invert(void) {
    a = -a;
    b = -b;
    c = -c;
    d = -d;
}

void ClipPlane::activate(const Plane& p) {
	if (!p_stack.empty())
		glDisable(GL_CLIP_PLANE0);

	p_stack.push_back(p);
	GLdouble pp[4] = {p.a, p.b, p.c, p.d};
	glClipPlane(GL_CLIP_PLANE0, pp);
	glEnable(GL_CLIP_PLANE0);
}

void ClipPlane::disable(void) {
	glDisable(GL_CLIP_PLANE0);
	p_stack.pop_back();

	if (!p_stack.empty()) {
		Plane& p = p_stack.back();
		GLdouble plane[4] = {p.a, p.b, p.c, p.d};
    	glClipPlane(GL_CLIP_PLANE0, plane);
    	glEnable(GL_CLIP_PLANE0);
	}
}
