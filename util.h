#ifndef UTIL_H
#define UTIL_H

#include "color.h"
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#define UNUSED(X) ((void)(X))

#define PI  (3.1415926535897932384626433832795)
#define ESC (27) /*escape key*/

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

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
	double width, height;
} textbox_t;

void *allocate(size_t sz);
double rad2deg(double rad);
double deg2rad(double deg);
void set_color(color_t color);
void draw_regular_polygon_filled(double x, double y, double orientation, double radius, shape_t shape, color_t color);
void draw_regular_polygon_line(double x, double y, double orientation, double radius, shape_t shape, double thickness, color_t color);
void draw_line(double x, double y, double angle, double magnitude, double thickness, color_t color);
int draw_text(color_t color, double x, double y, const char *fmt, ...);
int vdraw_text(color_t color, double x, double y, const char *fmt, va_list ap);

double random_float(void);

void fill_textbox(color_t color, textbox_t *t, const char *fmt, ...);
void draw_textbox(color_t color, textbox_t *t);

void draw_rectangle(double x, double y, double width, double height, color_t color, bool lines, double thickness);

double wrap_or_limit_x(double x);
double wrap_or_limit_y(double y);
double wraprad(double rad);

color_t team_to_color(unsigned team);

#endif
