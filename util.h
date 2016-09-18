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

typedef enum { /**@todo add pentagon, improve circle drawing*/
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

struct prng_t;
typedef struct prng_t prng_t;

typedef struct {
	double x, y;
	bool draw_box;
	double width, height;
} textbox_t;

void error(const char *fmt, ...);
void warning(const char *fmt, ...);
void *allocate(size_t sz);
double rad2deg(double rad);
double deg2rad(double deg);
void set_color(color_t color);
void draw_regular_polygon(double x, double y, double orientation, double radius, shape_t shape, color_t color);
void draw_line(double x, double y, double angle, double magnitude, double thickness, color_t color);
int draw_text(double x, double y, const char *fmt, ...);
int vdraw_text(double x, double y, const char *fmt, va_list ap);

prng_t *new_prng(uint64_t seed);
uint32_t prng(prng_t *state);
double prngf(prng_t *state);
void delete_prng(prng_t *state);

void fill_textbox(textbox_t *t, const char *fmt, ...);
void draw_textbox(textbox_t *t);

void draw_rectangle(double x, double y, double width, double height, color_t color, bool lines, double thickness);

double wrapx(double x);
double wrapy(double y);
double wraprad(double rad);

color_t team_to_color(unsigned team);

#endif
