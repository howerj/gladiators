/**@file  util.c
 * @brief generic C and Open GL utilities */
#include "util.h"
#include "vars.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <GL/glut.h>

#define FONT_HEIGHT (15)
#define FONT_WIDTH  (9)

struct prng_t {
	uint64_t seed;
};

void error(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
	exit(EXIT_FAILURE);
}

void warning(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
}

void *allocate(size_t sz)
{
	void *r = calloc(sz, 1);
	if(!r)
		error("allocation failed of size %zu\n", sz);
	return r;
}

double rad2deg(double rad)
{
	return (rad / (2.0 * PI)) * 360.0;
}

double deg2rad(double deg)
{
	return (deg / 360.0) * 2.0 * PI;
}

void set_color(color_t color)
{
	switch(color) {
	case WHITE:   glColor3f(1.0, 1.0, 1.0);   break;
	case RED:     glColor3f(0.8, 0.0, 0.0);   break;
	case YELLOW:  glColor3f(0.8, 0.8, 0.0);   break;
	case GREEN:   glColor3f(0.0, 0.8, 0.0);   break;
	case CYAN:    glColor3f(0.0, 0.8, 0.8);   break;
	case BLUE:    glColor3f(0.0, 0.0, 0.8);   break;
	case MAGENTA: glColor3f(0.8, 0.0, 0.8);   break;
	case BROWN:   glColor3f(0.35, 0.35, 0.0); break;
	case BLACK:   glColor3f(0.0, 0.0, 0.0);   break;
	default:
		error("invalid color '%d'", color);
	}
}

void draw_line(double x, double y, double angle, double magnitude, double thickness, color_t color)
{
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

/** see: https://www.opengl.org/discussion_boards/showthread.php/160784-Drawing-Circles-in-OpenGL
 * @todo Make line only version, instead of just having a filled polygon, also add in depth as an option */
static void _draw_regular_polygon(
		double x, double y, 
		double orientation, 
		double radius, double sides, 
		bool lines, double thickness, 
		color_t color)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();
		glTranslated(x, y, 0.0);
		glRotated(rad2deg(orientation), 0, 0, 1);
		set_color(color);
		if(lines) {
			glLineWidth(thickness);
			glBegin(GL_LINE_LOOP);
		} else {
			glBegin(GL_POLYGON);
		}
			for(double i = 0; i < 2 * PI; i += PI / sides)
				glVertex3d(cos(i) * radius, sin(i) * radius, 0.0);
		glEnd();
	glPopMatrix();
}

void draw_rectangle(double x, double y, double width, double height, color_t color, bool lines, double thickness)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();
		glRasterPos2d(x, y);
		set_color(color);
		if(lines) {
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

double shape_to_sides(shape_t shape)
{
	static const double sides[] = 
	{
		[TRIANGLE] = 1.5,
		[SQUARE]   = 2,
		[PENTAGON] = 2.5,
		[HEXAGON]  = 3,
		[SEPTAGON] = 3.5,
		[OCTAGON]  = 4,
		[DECAGON]  = 5,
		[CIRCLE]   = 24
	};
	if(shape > INVALID_SHAPE)
		error("invalid shape '%d'", shape);
	return sides[shape];
}

void draw_regular_polygon(double x, double y, double orientation, double radius, shape_t shape, color_t color)
{
	double sides = shape_to_sides(shape);
	_draw_regular_polygon(x, y, orientation, radius, sides, false, 0, color);
}

static uint32_t temper(uint32_t x)
{
    x ^= x>>11;
    x ^= x<<7 & 0x9D2C5680;
    x ^= x<<15 & 0xEFC60000;
    x ^= x>>18;
    return x;
}

/* from: https://stackoverflow.com/questions/19083566 */
static uint32_t lcg64_temper(uint64_t *seed)
{
    *seed = 6364136223846793005ULL * *seed + 1;
    return temper(*seed >> 32);
}

prng_t *new_prng(uint64_t seed)
{
	prng_t *r = allocate(sizeof(prng_t));
	r->seed = seed;
	return r;
}

uint32_t prng(prng_t *state)
{
	return lcg64_temper(&state->seed);
}

double prngf(prng_t *state) 
{
	double r = prng(state);
	r /= UINT32_MAX;
	return r;
}

void delete_prng(prng_t *state)
{
	free(state);
}

/* see: https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01
 *      https://stackoverflow.com/questions/538661/how-do-i-draw-text-with-glut-opengl-in-c
 *      https://stackoverflow.com/questions/20866508/using-glut-to-simply-print-text 
 *
 * @todo Make printf like version, with format specifiers for color
 * @todo Make wrapper that keeps track of position, and optionally draws box
 * around text */
static int draw_string(const char *msg)
{  
	size_t len = strlen(msg);
	for(size_t i = 0; i < len; i++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, msg[i]);
	return len;
}

int vdraw_text(double x, double y, const char *fmt, va_list ap)
{
	char f;
	int r = 0;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	set_color(BLUE); /* just the default */
	/*glTranslated(x, y, 0);*/
	glRasterPos2d(x, y);
	while(*fmt) {
		if('%' != (f = *fmt++)) {
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, f);
			r++;
			continue;
		}
		switch(f = *fmt++) {
		case 'c': 
		{
			char x[2] = {0, 0};
			x[0] = va_arg(ap, int);
			r += draw_string(x);
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
		case 'f':
		{
			double f = va_arg(ap, double);
			char s[512] = {0};
			sprintf(s, "%.4f", f);
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

int draw_text(double x, double y, const char *fmt, ...)
{
	int r;
	va_list ap;
	va_start(ap, fmt);
	r = vdraw_text(x, y, fmt, ap);
	va_end(ap);
	return r;
}

void fill_textbox(textbox_t *t, const char *fmt, ...)
{
	double r;
	va_list ap;
	va_start(ap, fmt);
	r = vdraw_text(t->x, t->y - t->height, fmt, ap);
	va_end(ap);
	t->width = MAX(t->width, r); 
	t->height += ((Ymax / window_height) * FONT_HEIGHT); /*correct?*/
}

void draw_textbox(textbox_t *t)
{
	if(!(t->draw_box))
		return;
	/**@todo fix this */
	draw_rectangle(t->x, t->y-t->height, t->width, t->height, YELLOW, true, 0.5);
}

double wrapx(double x)
{
	if(x > Xmax)
		x = Xmin;
	if(x < Xmin)
		x = Xmax;
	return x;
}

double wrapy(double y)
{
	if(y > Ymax)
		y = Ymin;
	if(y < Ymin)
		y = Ymax;
	return y;
}

double wraprad(double rad)
{
	if(rad > 2*PI)
		rad = 0.0;
	if(rad < 0)
		rad = 2*PI;
	return rad;
}

