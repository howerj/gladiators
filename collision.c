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

bool detect_circle_circle_collision(
		double ax, double ay, double aradius,
		double bx, double by, double bradius)
{
	double dx = ax - bx;
	double dy = ay - by;
	double distance = sqrt(dx * dx + dy * dy);
	return (distance < (aradius + bradius));
}

static inline double square(double a)
{
	return a*a;
}

typedef enum {
	quadrant_0_0,
	quadrant_0_1,
	quadrant_1_0,
	quadrant_1_1,
} quadrant_e;

static inline quadrant_e quadrant(double angle)
{
	quadrant_e r = quadrant_0_0;
	if(angle < PI*0.5)
		r = quadrant_0_0;
	else if(angle < PI)
		r = quadrant_0_1;
	else if(angle < PI*1.5)
		r = quadrant_1_0;
	else
		r = quadrant_1_1;
	return r;
}

/* see: https://stackoverflow.com/questions/1073336/circle-line-segment-collision-detection-algorithm */
bool detect_line_circle_intersection(
		double orientation, bool use_orientation,
		double Ax, double Ay,
		double Bx, double By,
		double Cx, double Cy, double Cradius, color_t color)
{
	double lab = sqrt(square(Bx - Ax) + square(By - Ay));
	double Dx  = (Bx - Ax) / lab;
	double Dy  = (By - Ay) / lab;
	double t   = Dx * (Cx - Ax) + Dy * (Cy - Ay);
	double Ex  = t * Dx + Ax;
	double Ey  = t * Dy + Ay;
	double lec = sqrt(square(Ex - Cx) + square(Ey - Cy));

	if(lec < Cradius) { /* two hits on circle */
		double dt = sqrt(square(Cradius) - square(lec));
		double Fx = (t-dt)*Dx + Ax;
		double Fy = (t-dt)*Dy + Ay;
		double Gx = (t+dt)*Dx + Ax;
		double Gy = (t+dt)*Dy + Ay;
		bool rval = true;

		if(use_orientation) 
			switch(quadrant(orientation)) {
			case quadrant_0_0: rval = Ax < Fx && Gx < Bx; break;
			case quadrant_0_1: rval = Fx < Ax && Bx < Gx; break;
			case quadrant_1_0: rval = Fx < Ax && Bx < Gx; break;
			case quadrant_1_1: rval = Ax < Fx && Gx < Bx; break;
			default: error("invalid quadrant");
			}

		if(rval && draw_line_circle_collision) {
			draw_regular_polygon_filled(Fx, Fy, 0, 1.0, CIRCLE, color);
			draw_regular_polygon_filled(Gx, Gy, 0, 1.0, CIRCLE, color);
		}

		if(rval && draw_line_circle_debug_line) {
			draw_line(Ax, Ay, 
				orientation,
				sqrt(square(Ax - Cx) + square(Ay - Cy)), 0.5, MAGENTA);

		}
		return rval;
	} else if(lec == Cradius) { /* tangent */
		return false;
	} else { /* no hits */
		return false;
	}
}

/**@todo improve the performance of this program by using
 * detect_circle_circle_collision then working out the angle, possibly
 * @note the angle of the collision could also returned, if desired. */
bool detect_circle_cone_collision(
		double kx, double ky, double korientation, double ksweep, unsigned lines, double klen,
		double cx, double cy, double cradius, color_t color)
{
	unsigned j = 0;
	for(double i = korientation - (ksweep/2); j < lines; i += (ksweep/lines), j++) {
		double bx = kx + (klen * cos(i)); /**@note does not handle wrapping */
		double by = ky + (klen * sin(i));
		bool d = detect_line_circle_intersection(i, true, kx, ky, bx, by, cx, cy, cradius, color);
		if(d)
			return true;
	}
	return false;
}

