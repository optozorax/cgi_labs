/*
[ ] Сделать контейнер для всех данных вектора
[ ] Запилить закидывание порталов в вектор, и чтобы там учитывался номер противоположного потрала
*/

#include <fstream>
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spob/spob.h>

#include "plane.h"
#include "shader.h"
#include "framebuffer.h"

using namespace spob;

double cam_alpha = deg2rad(30);
double cam_beta = deg2rad(30);
double cam_distance = 5;
double pi = _SPOB_PI;
int w, h;

vec3 cam_pos;
GLuint texture;
unsigned char *textureData;
GLuint textures[2];

glm::mat4 getFromMatrix(const crd3& crd) {
	glm::mat4 result;
	result[0] = glm::vec4(crd.i.x, crd.j.x, crd.k.x, crd.pos.x);
	result[1] = glm::vec4(crd.i.y, crd.j.y, crd.k.y, crd.pos.y);
	result[2] = glm::vec4(crd.i.z, crd.j.z, crd.k.z, crd.pos.z);
	result[3] = glm::vec4(0, 0, 0, 1);
	return glm::transpose(result);
}

glm::mat4 getToMatrix(const crd3& crd) {
	return glm::inverse(getFromMatrix(crd));
}

struct Portal {
	void init() {
		teleport1 = getFromMatrix(crd2) * getToMatrix(crd1);
		teleport2 = getFromMatrix(crd1) * getToMatrix(crd2);
		for (auto& i : polygon) {
			polygon1.push_back(plane3(crd1).from(i));
			polygon2.push_back(plane3(crd2).from(i));
		}

		p1.a = crd1.k.x;
		p1.b = crd1.k.y;
		p1.c = crd1.k.z;
		p1.d = -dot(crd1.pos, crd1.k);

		p2.a = -crd2.k.x;
		p2.b = -crd2.k.y;
		p2.c = -crd2.k.z;
		p2.d = dot(crd2.pos, crd2.k);
	}

	std::vector<vec2> polygon;
	space3 crd1, crd2;

	std::vector<vec3> polygon1, polygon2;
	glm::mat4 teleport1, teleport2;
	Plane p1, p2;
};

std::vector<int> not_draw_stack(1, -1);
struct PortalToDraw
{
	std::vector<vec3> polygon;
	glm::mat4 teleport, teleport_inverse;
	Plane plane;

	int not_draw;
};

struct PolygonToDraw {
    std::vector<vec3> polygon;
    std::vector<vec2> texCoords;
    bool isTextured;
    GLuint texture;
    vec3 color;
};

std::vector<PolygonToDraw> polygons;
std::vector<PortalToDraw> portals;

void addPortal(const std::vector<vec2>& polygon, const space3& crd1, const space3& crd2, const vec3& clr1, const vec3& clr2) {
	PortalToDraw p1, p2;

	p1.teleport = getFromMatrix(crd2) * getToMatrix(crd1);
	p2.teleport = getFromMatrix(crd1) * getToMatrix(crd2);
	p1.teleport_inverse = glm::inverse(p1.teleport);
	p2.teleport_inverse = glm::inverse(p2.teleport);

	for (auto& i : polygon) {
		p2.polygon.push_back(plane3(crd1).from(i));
		p1.polygon.push_back(plane3(crd2).from(i));
	}

	p1.plane.a = crd1.k.x;
	p1.plane.b = crd1.k.y;
	p1.plane.c = crd1.k.z;
	p1.plane.d = -dot(crd1.pos, crd1.k);

	p2.plane.a = -crd2.k.x;
	p2.plane.b = -crd2.k.y;
	p2.plane.c = -crd2.k.z;
	p2.plane.d = dot(crd2.pos, crd2.k);

	p1.not_draw = portals.size()+1;
	p2.not_draw = portals.size();

	portals.push_back(p1);
	portals.push_back(p2);

	polygons.push_back({{}, {}, false, 0, clr1});
    for (auto& i : polygon)
		polygons.back().polygon.push_back(plane3(crd1).from(i) - crd1.k*0.001);

	polygons.push_back({{}, {}, false, 0, clr2});
	for (auto& i : polygon)
		polygons.back().polygon.push_back(plane3(crd2).from(i) + crd2.k*0.001);
}

const int depthMax = 3;

void drawScene(int depth);

void drawPortal(const PortalToDraw& portal, int depth) {
	if (depth > depthMax) return;

	FrameBuffer f(w, h);
    f.activate();
    ClipPlane::activate(portal.plane);
        glMatrixMode(GL_MODELVIEW);
        glMultMatrixf(&portal.teleport[0][0]);
            drawScene(depth+1);
        glMatrixMode(GL_MODELVIEW);
        glMultMatrixf(&portal.teleport_inverse[0][0]);
    ClipPlane::disable();
    f.disable();

    PolygonFramebufferDrawer::draw(f, portal.polygon);
}

static GLfloat floorVertices[4][3] = {

};

void drawScene(int depth) {
	if (depth > depthMax) return;

    FrameBuffer f(w, h), f1(w, h);
    f.activate();
    f.disable(false);

    for (int i = 0; i < portals.size(); ++i) {
    	if (not_draw_stack.back() != portals[i].not_draw) {
    		not_draw_stack.push_back(i);
    		f1.activate();
    		drawPortal(portals[i], depth);
    		f1.disable(false);
    		not_draw_stack.pop_back();

    		f.activate(false);
            FrameBufferMerger::merge(f, f1);
            f.disable(false);
    	}
    }

    FrameBufferDrawer::draw(f);

	/*if (depth == 1) glColor4f(1.0, 0.0, 0.0, 1.0);
    if (depth == 2) glColor4f(0.0, 1.0, 0.0, 1.0);
    if (depth == 3) glColor4f(0.0, 0.0, 1.0, 1.0);
    if (depth == 4) glColor4f(1.0, 1.0, 0.0, 1.0);
    if (depth == 5) glColor4f(1.0, 0.0, 1.0, 1.0);
    if (depth == 6) glColor4f(0.0, 1.0, 1.0, 1.0);

    glBegin(GL_POLYGON);
    glTexCoord2f(0.0, 0.0); glVertex3fv(floorVertices[0]);
    glTexCoord2f(0.0, 1.0); glVertex3fv(floorVertices[1]);
    glTexCoord2f(2, 1.0); glVertex3fv(floorVertices[2]);
    glTexCoord2f(2, 0.0); glVertex3fv(floorVertices[3]);
    glEnd();

    if (depth == 1) glColor4f(0.5, 0.0, 0.0, 1.0);
    if (depth == 2) glColor4f(0.0, 0.5, 0.0, 1.0);
    if (depth == 3) glColor4f(0.0, 0.0, 0.5, 1.0);
    if (depth == 4) glColor4f(0.5, 0.5, 0.0, 1.0);
    if (depth == 5) glColor4f(0.5, 0.0, 0.5, 1.0);
    if (depth == 6) glColor4f(0.0, 0.5, 0.5, 1.0);

    glutSolidTorus(0.3, 1.5, 10, 30);

    glBegin(GL_LINES);
	glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(10, 0, 0);
	glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 10, 0);
	glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 10);
	glEnd();*/
	for (auto& i : polygons) {
        if (i.isTextured) {
            glBindTexture(GL_TEXTURE_2D, i.texture);
            glBegin(GL_POLYGON);
            for (auto& j : i.polygon)
                glVertex3f(j.x, j.y, j.z);
            glEnd();
            glBindTexture(GL_TEXTURE_2D, 0);
        } else {
            glColor3f(i.color.x, i.color.y, i.color.z);
            glBegin(GL_POLYGON);
            for (auto& j : i.polygon)
                glVertex3f(j.x, j.y, j.z);
            glEnd();
        }
	}
}

void display() {
    glClearColor(0.3, 0.3, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	FrameBuffer f(w, h);
	f.activate();
	drawScene(1);
	f.disable();
	FrameBufferDrawer::draw(f);
	glutSwapBuffers();
}

void update_cam(void) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, double(w)/h, 1.0, 1000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	vec3 pos1(sin(pi/2 - cam_beta) * cos(cam_alpha),
			  cos(pi/2 - cam_beta),
			  sin(pi/2 - cam_beta) * sin(cam_alpha));
	pos1 *= cam_distance;
	pos1 += cam_pos;
	gluLookAt(pos1.x, pos1.y, pos1.z,
			  cam_pos.x, cam_pos.y, cam_pos.z,
			  0, 1, 0);
}

void reshape(int w1, int h1) {
	w = w1; h = h1;
	glViewport(0, 0, w, h);
	update_cam();
	glutPostRedisplay();
}

int r_moving, r_startx, r_starty;
int l_moving, l_startx, l_starty;
int m_moving, m_startx, m_starty;
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			l_moving = 1;
			l_startx = x;
			l_starty = y;
		}
		if (state == GLUT_UP) {
			l_moving = 0;
		}
	}
	if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN) {
			r_moving = 1;
			r_startx = x;
			r_starty = y;
		}
		if (state == GLUT_UP) {
			r_moving = 0;
		}
	}
	if (button == GLUT_MIDDLE_BUTTON) {
		if (state == GLUT_DOWN) {
			m_moving = 1;
			m_startx = x;
			m_starty = y;
		}
		if (state == GLUT_UP) {
			m_moving = 0;
		}
	}
}

/* ARGSUSED1 */
void motion(int x, int y) {
	if (l_moving) {
		cam_alpha = deg2rad(rad2deg(cam_alpha) + 0.5*(x - l_startx));
		cam_beta = deg2rad(rad2deg(cam_beta) + 0.5*(y - l_starty));
		l_startx = x;
		l_starty = y;
		update_cam();
		glutPostRedisplay();
	}
	if (r_moving) {
		cam_pos.x -= 0.1*(x-r_startx);
		cam_pos.z -= 0.1*(y-r_starty);
		r_startx = x;
		r_starty = y;
		update_cam();
		glutPostRedisplay();
	}
	if (m_moving) {
		cam_pos.y += 0.1*(y-m_starty);
		m_starty = y;
		update_cam();
		glutPostRedisplay();
	}
}

void init() {
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glColor3f(1.0, 1.0, 1.0);

	// init portals
	std::vector<vec2> pPortal;
	pPortal.push_back({-1, -1});
	pPortal.push_back({-1, 1});
	pPortal.push_back({1, 1});
	pPortal.push_back({1, -1});

	crd3 cPortal1;
	cPortal1.i = vec3(1, 0, 0);
	cPortal1.j = vec3(0, 1, 0);
	cPortal1.k = vec3(0, 0, 1);

	cPortal1.pos = -cPortal1.k * 2.5;

	crd3 cPortal2 = cPortal1;
	cPortal2.pos = cPortal1.k * 2.5;

	addPortal(pPortal, cPortal1, cPortal2, vec3(1, 0.5, 0.15), vec3(0.1, 0.55, 1));

	polygons.push_back({{}, {}, false, 0, vec3(0.5, 0, 0)});
	polygons.back().polygon.push_back({-5.0, -5.0, 5.0});
	polygons.back().polygon.push_back({5.0, -5.0, 5.0});
	polygons.back().polygon.push_back({5.0, -5.0, -5.0});
	polygons.back().polygon.push_back({-5.0, -5.0, -5.0});

	polygons.push_back({{}, {}, false, 0, vec3(0, 0.5, 0)});
	polygons.back().polygon.push_back({-5.0, 5.0, 5.0});
	polygons.back().polygon.push_back({5.0, 5.0, 5.0});
	polygons.back().polygon.push_back({5.0, 5.0, -5.0});
	polygons.back().polygon.push_back({-5.0, 5.0, -5.0});

	polygons.push_back({{}, {}, false, 0, vec3(0, 0, 0.5)});
	polygons.back().polygon.push_back({-5.0, 5.0, 5.0});
	polygons.back().polygon.push_back({5.0, 5.0, 5.0});
	polygons.back().polygon.push_back({5.0, -5.0, 5.0});
	polygons.back().polygon.push_back({-5.0, -5.0, 5.0});

	polygons.push_back({{}, {}, false, 0, vec3(0, 0.5, 0.5)});
	polygons.back().polygon.push_back({-5.0, 5.0, -5.0});
	polygons.back().polygon.push_back({5.0, 5.0, -5.0});
	polygons.back().polygon.push_back({5.0, -5.0, -5.0});
	polygons.back().polygon.push_back({-5.0, -5.0, -5.0});

	polygons.push_back({{}, {}, false, 0, vec3(0.5, 0, 0.5)});
	polygons.back().polygon.push_back({-5.0, 5.0, -5.0});
	polygons.back().polygon.push_back({-5.0, 5.0, 5.0});
	polygons.back().polygon.push_back({-5.0, -5.0, 5.0});
	polygons.back().polygon.push_back({-5.0, -5.0, -5.0});

	polygons.push_back({{}, {}, false, 0, vec3(0.5, 0.5, 0)});
	polygons.back().polygon.push_back({5.0, 5.0, -5.0});
	polygons.back().polygon.push_back({5.0, 5.0, 5.0});
	polygons.back().polygon.push_back({5.0, -5.0, 5.0});
	polygons.back().polygon.push_back({5.0, -5.0, -5.0});

	std::vector<vec2> pPoly;
	pPoly.push_back({0, 0});
	pPoly.push_back({-0.4, 0.7});
	pPoly.push_back({0.4, 0.7});

	plane3 cPoly;
	cPoly.pos = vec3(0.5, 0.5, 0.5);
	cPoly.i = vec3(1, 0, 0);
	cPoly.k = vec3(0, 1, 0);
	cPoly.j = vec3(0, 0, 1);

	polygons.push_back({{}, {}, false, 0, vec3(0.5, 0.5, 0.5)});
	polygons.back().polygon.push_back(cPoly.from(pPoly[0]));
	polygons.back().polygon.push_back(cPoly.from(pPoly[1]));
	polygons.back().polygon.push_back(cPoly.from(pPoly[2]));

	update_cam();

	glutPostRedisplay();
}

void wheel(int button, int dir, int x, int y) {
	if (dir < 0) cam_distance += 0.5;
	else cam_distance -= 0.5;
	update_cam();
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 'a') cam_alpha = deg2rad(rad2deg(cam_alpha) + 3);
	if (key == 'o') cam_alpha = deg2rad(rad2deg(cam_alpha) - 3);

	if (key == 'e') cam_beta = deg2rad(rad2deg(cam_beta) + 3);
	if (key == 'u') cam_beta = deg2rad(rad2deg(cam_beta) - 3);

	if (key == '{') wheel(0, 1, 0, 0);
	if (key == '}') wheel(0, -1, 0, 0);

	update_cam();

	glutPostRedisplay();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize(800, 600);
	glutCreateWindow("A Simple Torus");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_TEXTURE_2D);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		return -1;
	}

	init();
	glutMainLoop();

	glDeleteTextures(1, &texture);
}
