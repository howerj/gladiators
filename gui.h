/** @file       gui.h
 *  @brief      GUI function wrappers
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/
#ifndef GUI_H

#include "color.h"
#include <stdbool.h>
#include <stdarg.h>

typedef enum {
	TRIANGLE,
	SQUARE,
	PENTAGON,
	HEXAGON,
	SEPTAGON,
	OCTAGON,
	DECAGON,
	CIRCLE,
	INVALID_SHAPE
} shape_e;

typedef shape_e shape_t;

typedef struct {
	double x, y;
	bool draw_box;
	color_t color_text, color_box;
	double width, height;
} textbox_t;

typedef struct { /**@note it might be worth translating some functions to use points*/
	double x, y;
} point_t;

void set_color(color_t color);
void draw_regular_polygon_filled(double x, double y, double orientation, double radius, shape_t shape, color_t color);
void draw_regular_polygon_line(double x, double y, double orientation, double radius, shape_t shape, double thickness, color_t color);
void draw_line(double x, double y, double angle, double magnitude, double thickness, color_t color);
void draw_cross(double x, double y, double angle, double magnitude, double thickness, color_t color);
void draw_arc_filled(double x, double y, double angle, double magnitude, double arc, color_t color);
void draw_arc_line(double x, double y, double angle, double magnitude, double arc, double thickness, color_t color);
void draw_rectangle_filled(double x, double y, double width, double height, color_t color);
void draw_rectangle_line(double x, double y, double width, double height, double thickness, color_t color);
int draw_text(color_t color, double x, double y, const char *fmt, ...);
int vdraw_text(color_t color, double x, double y, const char *fmt, va_list ap);

color_t team_to_color(unsigned team);

void fill_textbox(textbox_t *t, bool on, const char *fmt, ...);
void draw_textbox(textbox_t *t);

#endif