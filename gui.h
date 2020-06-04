/** @file       gui.h
 *  @brief      GUI function wrappers
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/
#ifndef GUI_H

#include "color.h"
#include <stdbool.h>
#include <stdarg.h>

enum {
	KEY_F1         =  0x4001,
	KEY_F2         =  0x4002,
	KEY_F3         =  0x4003,
	KEY_F4         =  0x4004,
	KEY_F5         =  0x4005,
	KEY_F6         =  0x4006,
	KEY_F7         =  0x4007,
	KEY_F8         =  0x4008,
	KEY_F9         =  0x4009,
	KEY_F10        =  0x400A,
	KEY_F11        =  0x400B,
	KEY_F12        =  0x400C,
	KEY_LEFT       =  0x400D,
	KEY_UP         =  0x400E,
	KEY_RIGHT      =  0x400F,
	KEY_DOWN       =  0x4010,
	KEY_PAGE_UP    =  0x4011,
	KEY_PAGE_DOWN  =  0x4012,
	KEY_HOME       =  0x4013,
	KEY_END        =  0x4014,
	KEY_INSERT     =  0x4015,
};

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

typedef struct {
	double x, y;
} point_t;

typedef struct {
	double xmin, xmax, ymin, ymax;
} resize_t;

typedef struct {
	const char *name;
	int start_x, start_y;
	int width, height;
	int timer_rate_ms;
	int (*keyboard)(void *kb_param, int key, int x, int y, int down);
	int (*draw)(void *draw_param);
	int (*timer)(void *timer_param, int value);
	int (*resize)(void *resize_param, int w, int h, resize_t *out);
	void *keyboard_param, *draw_param, *timer_param, *resize_param;
} gui_t;

void set_color(const color_t *color);
void draw_regular_polygon_filled(double x, double y, double orientation, double radius, shape_t shape, const color_t *color);
void draw_regular_polygon_line(double x, double y, double orientation, double radius, shape_t shape, double thickness, const color_t *color);
void draw_line(double x, double y, double angle, double magnitude, double thickness, const color_t *color);
void draw_cross(double x, double y, double angle, double magnitude, double thickness, const color_t *color);
void draw_arc_filled(double x, double y, double angle, double magnitude, double arc, const color_t *color);
void draw_arc_line(double x, double y, double angle, double magnitude, double arc, double thickness, const color_t *color);
void draw_rectangle_filled(double x, double y, double width, double height, const color_t *color);
void draw_rectangle_line(double x, double y, double width, double height, double thickness, const color_t *color);
int draw_text(const color_t *color, double x, double y, const char *fmt, ...);
int vdraw_text(const color_t *color, double x, double y, const char *fmt, va_list ap);

const color_t *team_to_color(unsigned team);

void fill_textbox(textbox_t *t, bool on, const char *fmt, ...);
void draw_textbox(textbox_t *t);

int gui_run(gui_t *gui);
int gui_elapsed_time(void);

#endif
