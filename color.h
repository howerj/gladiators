/** @file       color.h
 *  @brief      possible colors
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

#ifndef COLOR_H
#define COLOR_H

typedef struct {
	double r, g, b, a;
} color_t;

extern const color_t color_white;
extern const color_t color_red;
extern const color_t color_yellow;
extern const color_t color_green;
extern const color_t color_cyan;
extern const color_t color_blue;
extern const color_t color_magenta;
extern const color_t color_brown;
extern const color_t color_black;

extern const color_t *WHITE;
extern const color_t *RED;
extern const color_t *YELLOW;
extern const color_t *GREEN;
extern const color_t *CYAN;
extern const color_t *BLUE;
extern const color_t *MAGENTA;
extern const color_t *BROWN;
extern const color_t *BLACK;


#endif
