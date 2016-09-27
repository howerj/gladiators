/** @file       color.h
 *  @brief      possible colors
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

#ifndef COLOR_H
#define COLOR_H

/**@todo Make RGB struct */

typedef enum {
	WHITE,
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	BROWN,
	BLACK,
	INVALID_COLOR
} colors_e;

typedef colors_e color_t;

#endif
