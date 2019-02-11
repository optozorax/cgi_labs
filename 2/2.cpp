// This is a simple introductory program; its main window contains a static
// picture of a torus.  The program illustrates viewing by choosing a camera
// setup with gluLookAt(), which is conceptually simpler than transforming
// objects to move into a predefined view volume.

#include <GL/freeglut.h>
#include <spob/spob.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace spob;

double cam_alpha = deg2rad(30);
double cam_beta = deg2rad(30);
double cam_distance = 50;
double pi = _SPOB_PI;

vec3 cam_pos;
GLuint texture;
unsigned char *textureData;
GLuint textures[2];

// Clears the window and draws the torus.
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw a white torus of outer radius 3, inner radius 0.5 with 15 stacks and 30 slices.
	glColor3f(1.0, 1.0, 1.0);
	glutWireTorus(0.5, 3, 15, 30);
	//glutSolidTorus(0.5, 3, 15, 30);

	// Оси
	glBegin(GL_LINES);
	glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(10, 0, 0);
	glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 10, 0);
	glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 10);
	glEnd();

	glColor3f(1, 1, 1);
	glPointSize(6);
	glBegin(GL_POINTS);
	glVertex3f(cam_pos.x, cam_pos.y, cam_pos.z);
	glEnd();

	// Рисуем пол из текстуры
	auto drawFloor = [&] () {
		static GLfloat floorVertices[4][3] = {
			{ -20.0, 0.0, 20.0 },
			{ 20.0, 0.0, 20.0 },
			{ 20.0, 0.0, -20.0 },
			{ -20.0, 0.0, -20.0 },
		};

		glBindTexture(GL_TEXTURE_2D, textures[0]);

		glBegin(GL_POLYGON);
		const double plus = 0;
		glTexCoord2f(0.0+plus, 0.0+plus); glVertex3fv(floorVertices[0]);
		glTexCoord2f(0.0+plus, 1.0-plus); glVertex3fv(floorVertices[1]);
		glTexCoord2f(1.0-plus, 1.0-plus); glVertex3fv(floorVertices[2]);
		glTexCoord2f(1.0-plus, 0.0+plus); glVertex3fv(floorVertices[3]);
		/*glTexCoord2f(0.0, 0.0); glVertex3fv(floorVertices[0]);
		glTexCoord2f(0.0, 1.0); glVertex3fv(floorVertices[1]);
		glTexCoord2f(2, 1.0); glVertex3fv(floorVertices[2]);
		glTexCoord2f(2, 0.0); glVertex3fv(floorVertices[3]);*/
		glEnd();
	};
	drawFloor();
	//glFrontFace(GL_CW);
	//drawFloor();
	//glFrontFace(GL_CCW);

	auto drawUp = [&] () {
		static GLfloat floorVertices[4][3] = {
			{ -20.0, -3.0, 20.0 },
			{ 20.0, -3.0, 20.0 },
			{ 20.0, 3.0, -20.0 },
			{ -20.0, 3.0, -20.0 },
		};

		glBindTexture(GL_TEXTURE_2D, textures[0]);

		glBegin(GL_POLYGON);
		const double plus = -1;
		glTexCoord2f(0.0+plus, 0.0+plus); glVertex3fv(floorVertices[0]);
		glTexCoord2f(0.0+plus, 1.0-plus); glVertex3fv(floorVertices[1]);
		glTexCoord2f(1.0-plus, 1.0-plus); glVertex3fv(floorVertices[2]);
		glTexCoord2f(1.0-plus, 0.0+plus); glVertex3fv(floorVertices[3]);
		/*glTexCoord2f(0.0, 0.0); glVertex3fv(floorVertices[0]);
		glTexCoord2f(0.0, 1.0); glVertex3fv(floorVertices[1]);
		glTexCoord2f(2, 1.0); glVertex3fv(floorVertices[2]);
		glTexCoord2f(2, 0.0); glVertex3fv(floorVertices[3]);*/
		glEnd();
	};
	drawUp();
	//glFrontFace(GL_CW);
	//drawUp();
	//glFrontFace(GL_CCW);

	glutSwapBuffers();
}

int w, h;

void update_cam(void) {
	// Set the camera lens to have a 60 degree (vertical) field of view, an
	// aspect ratio of 4/3, and have everything closer than 1 unit to the
	// camera and greater than 40 units distant clipped away.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, double(w)/h, 1.0, 1000.0);

	// Position camera at (4, 6, 5) looking at (0, 0, 0) with the vector
	// <0, 1, 0> pointing upward.
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

// Sets up global attributes like clear color and drawing color, and sets up
// the desired projection and modelview matrices.
void init() {
	// Set the current clear color to black and the current drawing color to
	// white.
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glColor3f(1.0, 1.0, 1.0);	

	// Загружаем текстуры
	/* Nice floor texture tiling pattern. */
	const char *circles[] = {
		"....xxxx........",
		"..xxxxxxxx......",
		".xxxxxxxxxx.....",
		".xxx....xxx.....",
		"xxx......xxx....",
		"xxx..xx..xxx....",
		"xxx..xx..xxx....",
		"xxx......xxx....",
		".xxx....xxx.....",
		".xxxxxxxxxx.....",
		"..xxxxxxxx......",
		"....xxxx...x....",
		"............x...",
		".............x..",
		"..............x.",
		"...............x",
	};
	GLubyte floorTexture[16][16][3];
	GLubyte *loc;
	int s, t;

	/* Setup RGB image for the texture. */
	loc = (GLubyte*) floorTexture;
	for (t = 0; t < 16; t++) {
		for (s = 0; s < 16; s++) {
			if (circles[t][s] == 'x') {
				/* Nice green. */
				loc[0] = 0x1f;
				loc[1] = 0x8f;
				loc[2] = 0x1f;
			} else {
				/* Light gray. */
				loc[0] = 0xaa;
				loc[1] = 0xaa;
				loc[2] = 0xaa;
			}
			loc += 3;
		}
	}

	unsigned char *textureData;
	glGenTextures(2, textures);
	int width, height, n;
	textureData = stbi_load("texture.png", &width, &height, &n, 3);

	glBindTexture(GL_TEXTURE_2D, textures[0]);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, textureData);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);

	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, floorTexture);

	update_cam();

	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 'a') cam_alpha = deg2rad(rad2deg(cam_alpha) + 3);
	if (key == 'o') cam_alpha = deg2rad(rad2deg(cam_alpha) - 3);

	if (key == 'e') cam_beta = deg2rad(rad2deg(cam_beta) + 3);
	if (key == 'u') cam_beta = deg2rad(rad2deg(cam_beta) - 3);

	update_cam();

	glutPostRedisplay();
}

void wheel(int button, int dir, int x, int y) {
	if (dir < 0) cam_distance += 0.5;
	else cam_distance -= 0.5;
	update_cam();
	glutPostRedisplay();
}


// Initializes GLUT, the display mode, and main window; registers callbacks;
// does application initialization; enters the main event loop.
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize(800, 600);
	glutCreateWindow("A Simple Torus");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);
	glutMouseWheelFunc(wheel);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	init();
	glutMainLoop();

	// не забываем освободить память
	stbi_image_free(textureData);

	// не забываем освободить их при выходе из скоупа
	glDeleteTextures(1, &texture);
}