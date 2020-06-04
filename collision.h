/** @file       collision.h
 *  @brief      collision detection algorithms
 *  @author     Richard James Howe (2016, 2020)
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

double euclidean_distance(double ax, double ay, double bx, double by);

#endif
