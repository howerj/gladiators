/** @file       gui.c
 *  @brief      GUI function wrappers
 *  @author     Richard James Howe (2016, 2020)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

#include "gui.h"
#include "util.h" /* TODO: Remove as a dependency */
#include "vars.h" /* TODO: Remove as a dependency */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <GL/glut.h>

/* TODO: A completely headless version of the system should be
 * made, that can be compiled without the GUI libraries, perhaps
 * rendering could be to video instead...*/
/* TODO: Make it so we can customize this, along with font? */
#define FONT_HEIGHT (15)
#define FONT_WIDTH  (9)

static int gui_set = 0;
static gui_t gui = { .timer_param = NULL, };

const color_t color_white   = { .r = 1.0,  .g = 1.0,  .b = 1.0, .a = 1.0 };
const color_t color_red     = { .r = 0.8,  .g = 0.0,  .b = 0.0, .a = 1.0 };
const color_t color_yellow  = { .r = 0.8,  .g = 0.8,  .b = 0.0, .a = 1.0 };
const color_t color_green   = { .r = 0.0,  .g = 0.8,  .b = 0.0, .a = 1.0 };
const color_t color_cyan    = { .r = 0.0,  .g = 0.8,  .b = 0.8, .a = 1.0 };
const color_t color_blue    = { .r = 0.0,  .g = 0.0,  .b = 0.8, .a = 1.0 };
const color_t color_magenta = { .r = 0.8,  .g = 0.0,  .b = 0.8, .a = 1.0 };
const color_t color_brown   = { .r = 0.35, .g = 0.35, .b = 0.0, .a = 1.0 };
const color_t color_black   = { .r = 0.0,  .g = 0.0,  .b = 0.0, .a = 1.0 };

const color_t *WHITE   = &color_white;
const color_t *RED     = &color_red;
const color_t *YELLOW  = &color_yellow;
const color_t *GREEN   = &color_green;
const color_t *CYAN    = &color_cyan;
const color_t *BLUE    = &color_blue;
const color_t *MAGENTA = &color_magenta;
const color_t *BROWN   = &color_brown;
const color_t *BLACK   = &color_black;

void set_color(const color_t *color) {
	glColor3f(color->r, color->g, color->b);
}

/* TODO: Move to a different module */
const color_t *team_to_color(unsigned team) {
	static const color_t *colors[] = { &color_red, &color_green, &color_yellow, &color_cyan, &color_blue, &color_magenta };
	if (team >= sizeof(colors) / sizeof(colors[0])) {
		static bool warned = false; /*not thread-safe, although of no real consequence*/
		if (!warned) {
			fprintf(stderr, "warning: gladiator -- ran out of team colors %u", team);
			warned = true;
		}
		return &color_magenta;
	}
	return colors[team];
}

void draw_line(double x, double y, double angle, double magnitude, double thickness, const color_t *color) {
	if (gui_set == 0)
		return;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();
		glTranslated(x, y, 0);
		glRotated(rad2deg(angle), 0, 0, 1);
		glLineWidth(thickness);
		set_color(color);
		glBegin(GL_LINES);
			glVertex3d(0, 0, 0);
			glVertex3d(magnitude, 0, 0);
		glEnd();
	glPopMatrix();
}

void draw_cross(double x, double y, double angle, double magnitude, double thickness, const color_t *color) {
	if (gui_set == 0)
		return;
	double xn = x-cos(angle)*(magnitude/2);
	double yn = y-sin(angle)*(magnitude/2);
	draw_line(xn, yn, angle, magnitude, thickness, color);
	xn = x-cos(angle+PI/2)*(magnitude/2);
	yn = y-sin(angle+PI/2)*(magnitude/2);
	draw_line(xn, yn, angle+PI/2, magnitude, thickness, color);
}

static void draw_arc(double x, double y, double angle, double magnitude, double arc, bool lines, double thickness, const color_t *color) {
	if (gui_set == 0)
		return;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();
		glTranslated(x, y, 0.0);
		glRotated(rad2deg(angle), 0, 0, 1);
		set_color(color);
		if (lines) {
			glLineWidth(thickness);
			glBegin(GL_LINE_LOOP);
		} else {
			glBegin(GL_POLYGON);
		}
			glVertex3d(0, 0, 0.0);
			for (double i = 0; i < arc; i += arc / 24.0)
				glVertex3d(cos(i) * magnitude, sin(i) * magnitude, 0.0);
		glEnd();
	glPopMatrix();
}

void draw_arc_filled(double x, double y, double angle, double magnitude, double arc, const color_t *color) {
	return draw_arc(x, y, angle, magnitude, arc, false, 0, color);
}

void draw_arc_line(double x, double y, double angle, double magnitude, double arc, double thickness, const color_t *color) {
	return draw_arc(x, y, angle, magnitude, arc, true, thickness, color);
}

/* see: https://www.opengl.org/discussion_boards/showthread.php/160784-Drawing-Circles-in-OpenGL */
static void draw_regular_polygon(
		double x, double y,
		double orientation,
		double radius, double sides,
		bool lines, double thickness,
		const color_t *color) {
	if (gui_set == 0)
		return;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();
		glTranslated(x, y, 0.0);
		glRotated(rad2deg(orientation), 0, 0, 1);
		set_color(color);
		if (lines) {
			glLineWidth(thickness);
			glBegin(GL_LINE_LOOP);
		} else {
			glBegin(GL_POLYGON);
		}
			for (double i = 0; i < 2.0 * PI; i += PI / sides)
				glVertex3d(cos(i) * radius, sin(i) * radius, 0.0);
		glEnd();
	glPopMatrix();
}

static void _draw_rectangle(double x, double y, double width, double height, bool lines, double thickness, const color_t *color) {
	if (gui_set == 0)
		return;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();
		glRasterPos2d(x, y);
		set_color(color);
		if (lines) {
			glLineWidth(thickness);
			glBegin(GL_LINE_LOOP);
		} else {
			glBegin(GL_POLYGON);
		}
		glVertex3d(x,       y,        0);
		glVertex3d(x+width, y,        0);
		glVertex3d(x+width, y+height, 0);
		glVertex3d(x,       y+height, 0);
		glEnd();
	glPopMatrix();
}

void draw_rectangle_filled(double x, double y, double width, double height, const color_t *color) {
	return _draw_rectangle(x, y, width, height, false, 0, color);
}

void draw_rectangle_line(double x, double y, double width, double height, double thickness, const color_t *color) {
	return _draw_rectangle(x, y, width, height, true, thickness, color);
}

double shape_to_sides(shape_t shape) {
	static const double sides[] = {
		[TRIANGLE] = 1.5,
		[SQUARE]   = 2,
		[PENTAGON] = 2.5,
		[HEXAGON]  = 3,
		[SEPTAGON] = 3.5,
		[OCTAGON]  = 4,
		[DECAGON]  = 5,
		[CIRCLE]   = 24
	};
	if (shape > INVALID_SHAPE)
		error("invalid shape '%d'", shape);
	return sides[shape % INVALID_SHAPE];
}

void draw_regular_polygon_filled(double x, double y, double orientation, double radius, shape_t shape, const color_t *color) {
	if (gui_set == 0)
		return;
	double sides = shape_to_sides(shape);
	draw_regular_polygon(x, y, orientation, radius, sides, false, 0, color);
}

void draw_regular_polygon_line(double x, double y, double orientation, double radius, shape_t shape, double thickness, const color_t *color) {
	if (gui_set == 0)
		return;
	double sides = shape_to_sides(shape);
	draw_regular_polygon(x, y, orientation, radius, sides, true, thickness, color);
}

/* see: https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01
 *      https://stackoverflow.com/questions/538661/how-do-i-draw-text-with-glut-opengl-in-c
 *      https://stackoverflow.com/questions/20866508/using-glut-to-simply-print-text */
static int draw_string(const char *msg) {
	assert(msg);
	if (gui_set == 0)
		return 0;
	size_t len = strlen(msg);
	for (size_t i = 0; i < len; i++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, msg[i]);
	return len;
}

int vdraw_text(const color_t *color, double x, double y, const char *fmt, va_list ap) {
	int r = 0;
	assert(fmt);
	if (gui_set == 0)
		return 0;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	set_color(color);
	/*glTranslated(x, y, 0);*/
	glRasterPos2d(x, y);
	while (*fmt) {
		char f = 0;
		if ('%' != (f = *fmt++)) {
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, f);
			r++;
			continue;
		}
		switch (f = *fmt++) {
		case 'c':
		{
			char vl[2] = {0, 0};
			vl[0] = va_arg(ap, int);
			r += draw_string(vl);
			break;
		}
		case 's':
		{
			char *s = va_arg(ap, char*);
			r += draw_string(s);
			break;
		}
		case 'u':
		case 'd':
		{
			int d = va_arg(ap, int);
			char s[64] = {0};
			sprintf(s, f == 'u' ? "%u": "%d", d);
			r += draw_string(s);
			break;
		}
		case 'g':
		{
			double flt = va_arg(ap, double);
			char s[512] = {0};
			sprintf(s, "%g", flt);
			r += draw_string(s);
			break;
		}
		case 'f':
		{
			double flt = va_arg(ap, double);
			char s[512] = {0};
			sprintf(s, "%.2f", flt);
			r += draw_string(s);
			break;
		}
		case 0:
		default:
			error("invalid format specifier '%c'", f);
		}

	}
	glPopMatrix();
	return r;
}

int draw_text(const color_t *color, double x, double y, const char *fmt, ...) {
	assert(fmt);
	if (gui_set == 0)
		return 0 ;
	int r;
	va_list ap;
	va_start(ap, fmt);
	r = vdraw_text(color, x, y, fmt, ap);
	va_end(ap);
	return r;
}

void fill_textbox(textbox_t *t, bool on, const char *fmt, ...) {
	double r;
	va_list ap;
	assert(t && fmt);
	if (!on || gui_set == 0)
		return;
	va_start(ap, fmt);
	r = vdraw_text(&t->color_text, t->x, t->y - t->height, fmt, ap);
	va_end(ap);
	t->width = MAX(t->width, r);
	t->height += ((Ymax / window_height) * FONT_HEIGHT); /*correct?*/
}

void draw_textbox(textbox_t *t) {
	assert(t);
	if (!(t->draw_box) || gui_set == 0)
		return;
	draw_rectangle_line(t->x, t->y-t->height, t->width, t->height, 0.5, &t->color_box);
}

static int specialk(int key) {
	if (key < 128)
		return key;
	switch (key) {
	case  GLUT_KEY_F1:         key  =  KEY_F1;         break;
	case  GLUT_KEY_F2:         key  =  KEY_F2;         break;
	case  GLUT_KEY_F3:         key  =  KEY_F3;         break;
	case  GLUT_KEY_F4:         key  =  KEY_F4;         break;
	case  GLUT_KEY_F5:         key  =  KEY_F5;         break;
	case  GLUT_KEY_F6:         key  =  KEY_F6;         break;
	case  GLUT_KEY_F7:         key  =  KEY_F7;         break;
	case  GLUT_KEY_F8:         key  =  KEY_F8;         break;
	case  GLUT_KEY_F9:         key  =  KEY_F9;         break;
	case  GLUT_KEY_F10:        key  =  KEY_F10;        break;
	case  GLUT_KEY_F11:        key  =  KEY_F11;        break;
	case  GLUT_KEY_F12:        key  =  KEY_F12;        break;
	case  GLUT_KEY_LEFT:       key  =  KEY_LEFT;       break;
	case  GLUT_KEY_UP:         key  =  KEY_UP;         break;
	case  GLUT_KEY_RIGHT:      key  =  KEY_RIGHT;      break;
	case  GLUT_KEY_DOWN:       key  =  KEY_DOWN;       break;
	case  GLUT_KEY_PAGE_UP:    key  =  KEY_PAGE_UP;    break;
	case  GLUT_KEY_PAGE_DOWN:  key  =  KEY_PAGE_DOWN;  break;
	case  GLUT_KEY_HOME:       key  =  KEY_HOME;       break;
	case  GLUT_KEY_END:        key  =  KEY_END;        break;
	case  GLUT_KEY_INSERT:     key  =  KEY_INSERT;     break;
	default:                   key  =  -1;             break;
	}
	return key;
}

static void keyboard_handler(unsigned char key, int x, int y) {
	assert(gui.keyboard);
	gui.keyboard(gui.keyboard_param, specialk(key), x, y, -1);
}

static void keyboard_special_down_handler(int key, int x, int y) {
	assert(gui.keyboard);
	gui.keyboard(gui.keyboard_param, specialk(key), x, y, 1);
}

static void keyboard_special_up_handler(int key, int x, int y) {
	assert(gui.keyboard);
	gui.keyboard(gui.keyboard_param, specialk(key), x, y, 0);
}

static void timer_callback(int value) {
	assert(gui.timer);
	gui.timer(gui.timer_param, value);
	glutTimerFunc(gui.timer_rate_ms, timer_callback, value);
}

static void draw_scene(void) {
	assert(gui.draw);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gui.draw(gui.draw_param);
	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}

static void resize_window(int w, int h) {
	assert(gui.resize);
	glViewport(0, 0, w, h);
	resize_t rsz = { 0, };
	gui.resize(gui.resize_param, w, h, &rsz);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(rsz.xmin, rsz.xmax, rsz.ymin, rsz.ymax, -1, 1);
}

int gui_run(gui_t *g) {
	assert(g);
	if (gui_set)
		return -1;
	if (g->draw == NULL)
		return -1;
	gui_set = 1;
	gui = *g;
	char *glut_argv[] = { (char*)g->name, NULL };
	int glut_argc = 0;
	glutInit(&glut_argc, glut_argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitWindowPosition(g->start_x, g->start_y);
	glutInitWindowSize(g->width, g->height);
	glutCreateWindow(g->name);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	if (g->keyboard)
		glutKeyboardFunc(keyboard_handler);
	if (g->keyboard)
		glutSpecialFunc(keyboard_special_down_handler);
	if (g->keyboard)
		glutSpecialUpFunc(keyboard_special_up_handler);
	glutReshapeFunc(resize_window);
	glutDisplayFunc(draw_scene);
	if (g->timer)
		glutTimerFunc(g->timer_rate_ms, timer_callback, 0);
	glutMainLoop();
	return 0;
}

int gui_elapsed_time(void) {
	if (gui_set == 0)
		return 0;
	return glutGet(GLUT_ELAPSED_TIME);
}


