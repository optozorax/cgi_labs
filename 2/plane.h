#pragma once

#include <vector>

struct Plane {
	double a, b, c, d;
	void invert(void);
};


class ClipPlane {
public:
	static void activate(const Plane& p);
	static void disable(void);
private:
	static std::vector<Plane> p_stack;
};