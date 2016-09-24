/** @file       util.h
 *  @brief      
 *  @author     Richard Howe (2016)
 *  @license    LGPL v2.1 or Later 
 *              <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html> 
 *  @email      howe.r.j.89@gmail.com*/

/**@todo draw arc and draw cross */

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
	color_t color_text, color_box;
	double width, height;
} textbox_t;

typedef struct { /**@note it might be worth translating some functions to use points*/
	double x, y;
} point_t;

typedef struct {
	unsigned i;
	unsigned max;
} tick_timer_t;

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

void fill_textbox(textbox_t *t, bool on, const char *fmt, ...);
void draw_textbox(textbox_t *t);

void draw_rectangle(double x, double y, double width, double height, color_t color, bool lines, double thickness);

double wrap_or_limit_x(double x);
double wrap_or_limit_y(double y);
double wrap_rad(double rad);

color_t team_to_color(unsigned team);
bool tick_timer(tick_timer_t *t);

#endif
