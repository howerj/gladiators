/** @file       collision.h
 *  @brief      collision detection algorithms
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com */

#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>
#include "color.h"

bool detect_circle_circle_collision(
		double ax, double ay, double aradius,
		double bx, double by, double bradius);

bool detect_line_circle_intersection(
		double orientation, bool use_orientation,
		double Ax, double Ay,
		double Bx, double By,
		double Cx, double Cy, double Cradius, color_t color);

bool detect_circle_cone_collision(
		double kx, double ky, double korientation, double ksweep, unsigned lines, double klen,
		double cx, double cy, double cradius, color_t color);

#endif
