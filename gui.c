/** @file       gui.c
 *  @brief      GUI function wrappers
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

#include "gui.h"
#include "util.h"
#include "vars.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <GL/glut.h>

#define FONT_HEIGHT (15)
#define FONT_WIDTH  (9)

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

color_t team_to_color(unsigned team)
{
	static const color_t colors[] = { RED, GREEN, YELLOW, CYAN, BLUE, MAGENTA };
	static bool warned = false; /**@warning not threadsafe, although of no real consequence*/
	if(team >= sizeof(colors)/sizeof(colors[0])) {
		if(!warned) {
			warning("gladiator: ran out of team colors %u", team);
			warned = true;
		}
		return MAGENTA;
	}
	return colors[team];
}

void draw_line(double x, double y, double angle, double magnitude, double thickness, color_t color)
{
	if(program_run_headless)
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

void draw_cross(double x, double y, double angle, double magnitude, double thickness, color_t color)
{
	double xn, yn;
	if(program_run_headless)
		return;
	xn = x-cos(angle)*(magnitude/2);
	yn = y-sin(angle)*(magnitude/2);
	draw_line(xn, yn, angle, magnitude, thickness, color);
	xn = x-cos(angle+PI/2)*(magnitude/2);
	yn = y-sin(angle+PI/2)*(magnitude/2);
	draw_line(xn, yn, angle+PI/2, magnitude, thickness, color);
}

static void _draw_arc(double x, double y, double angle, double magnitude, double arc, bool lines, double thickness, color_t color)
{
	if(program_run_headless)
		return;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();
		glTranslated(x, y, 0.0);
		glRotated(rad2deg(angle), 0, 0, 1);
		set_color(color);
		if(lines) {
			glLineWidth(thickness);
			glBegin(GL_LINE_LOOP);
		} else {
			glBegin(GL_POLYGON);
		}
			glVertex3d(0, 0, 0.0);
			for(double i = 0; i < arc; i += arc / 24.0)
				glVertex3d(cos(i) * magnitude, sin(i) * magnitude, 0.0);
		glEnd();
	glPopMatrix();

}

void draw_arc_filled(double x, double y, double angle, double magnitude, double arc, color_t color)
{
	return _draw_arc(x, y, angle, magnitude, arc, false, 0, color);
}

void draw_arc_line(double x, double y, double angle, double magnitude, double arc, double thickness, color_t color)
{
	return _draw_arc(x, y, angle, magnitude, arc, true, thickness, color);
}

/* see: https://www.opengl.org/discussion_boards/showthread.php/160784-Drawing-Circles-in-OpenGL */
static void _draw_regular_polygon(
		double x, double y, 
		double orientation, 
		double radius, double sides, 
		bool lines, double thickness, 
		color_t color)
{
	if(program_run_headless)
		return;
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
			for(double i = 0; i < 2.0 * PI; i += PI / sides)
				glVertex3d(cos(i) * radius, sin(i) * radius, 0.0);
		glEnd();
	glPopMatrix();
}

static void _draw_rectangle(double x, double y, double width, double height, bool lines, double thickness, color_t color)
{
	if(program_run_headless)
		return;
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

void draw_rectangle_filled(double x, double y, double width, double height, color_t color)
{
	return _draw_rectangle(x, y, width, height, false, 0, color); 
}

void draw_rectangle_line(double x, double y, double width, double height, double thickness, color_t color)
{
	return _draw_rectangle(x, y, width, height, true, thickness, color); 
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
	return sides[shape % INVALID_SHAPE];
}

void draw_regular_polygon_filled(double x, double y, double orientation, double radius, shape_t shape, color_t color)
{
	if(program_run_headless)
		return;
	double sides = shape_to_sides(shape);
	_draw_regular_polygon(x, y, orientation, radius, sides, false, 0, color);
}

void draw_regular_polygon_line(double x, double y, double orientation, double radius, shape_t shape, double thickness, color_t color)
{
	if(program_run_headless)
		return;
	double sides = shape_to_sides(shape);
	_draw_regular_polygon(x, y, orientation, radius, sides, true, thickness, color);
}

/* see: https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01
 *      https://stackoverflow.com/questions/538661/how-do-i-draw-text-with-glut-opengl-in-c
 *      https://stackoverflow.com/questions/20866508/using-glut-to-simply-print-text */
static int draw_string(const char *msg)
{	
	assert(msg);
	if(program_run_headless)
		return 0;
	size_t len = strlen(msg);
	for(size_t i = 0; i < len; i++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, msg[i]);	
	return len;
}

int vdraw_text(color_t color, double x, double y, const char *fmt, va_list ap)
{
	char f;
	int r = 0;
	assert(fmt);
	if(program_run_headless)
		return 0;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	set_color(color); 
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
			sprintf(s, "%.2f", f);
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

int draw_text(color_t color, double x, double y, const char *fmt, ...)
{
	assert(fmt);
	if(program_run_headless)
		return 0 ;
	int r;
	va_list ap;
	va_start(ap, fmt);
	r = vdraw_text(color, x, y, fmt, ap);
	va_end(ap);
	return r;
}

void fill_textbox(textbox_t *t, bool on, const char *fmt, ...)
{
	double r;
	va_list ap;
	assert(t && fmt);
	if(!on || program_run_headless)
		return;
	va_start(ap, fmt);
	r = vdraw_text(t->color_text, t->x, t->y - t->height, fmt, ap);
	va_end(ap);
	t->width = MAX(t->width, r); 
	t->height += ((Ymax / window_height) * FONT_HEIGHT); /*correct?*/
}

void draw_textbox(textbox_t *t)
{
	assert(t);
	if(!(t->draw_box) || program_run_headless)
		return;
	/**@todo fix this */
	draw_rectangle_line(t->x, t->y-t->height, t->width, t->height, 0.5, t->color_box);
}

double wrap_or_limit_x(double x)
{
	if(arena_wraps_at_edges) {
		if(x > Xmax)
			x = Xmin;
		if(x < Xmin)
			x = Xmax;
	} else {
		x = MIN(x, Xmax);
		x = MAX(x, Xmin);
	}
	return x;
}

double wrap_or_limit_y(double y)
{
	if(arena_wraps_at_edges) {
		if(y > Ymax)
			y = Ymin;
		if(y < Ymin)
			y = Ymax;
	} else {
		y = MIN(y, Ymax);
		y = MAX(y, Ymin);
	}
	return y;
}


