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

bool detect_circle_arc_collision(
		double kx, double ky, double korientation, double ksweep, double klen,
		double cx, double cy, double cradius);

#endif
