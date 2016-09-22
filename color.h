/** @file       color.h
 *  @brief      
 *  @author     Richard Howe (2016)
 *  @license    LGPL v2.1 or Later 
 *              <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html> 
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
