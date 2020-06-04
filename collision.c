/** @file       collision.c
 *  @brief      collision detection algorithms
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com */
#include "collision.h"
#include "util.h"
#include "vars.h"
#include "gui.h"
#include <math.h>

double euclidean_distance(double ax, double ay, double bx, double by) {
	const double dx = ax - bx;
	const double dy = ay - by;
	return hypot(dx, dy);
}

bool detect_circle_circle_collision(double ax, double ay, double aradius, double bx, double by, double bradius) {
	double dx = ax - bx;
	double dy = ay - by;
	double distance = hypot(dx, dy);
	return (distance < (aradius + bradius));
}

bool detect_circle_arc_collision(
		double kx, double ky, double korientation, double ksweep, double klen,
		double cx, double cy, double cradius) {
	double dx = kx - cx;
	double dy = ky - cy;
	double distance = hypot(dx, dy);
	if (!(distance < (klen + cradius)))
		return false;
	double angle = atan2(dy, dx) + PI;
	angle = wrap_rad(angle);
	ksweep /= 2.0;
	if ((angle < (korientation + ksweep)) && (angle > (korientation - ksweep))) {
		if (draw_circle_arc_debug_line)
			draw_line(kx, ky, angle, hypot(dx, dy), 0.5, MAGENTA);
		return true;
	}
	return false;
}

