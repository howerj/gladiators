#include "util.h"
#include "vars.h"
#include "wrap.h"

double wrap_or_limit_x(double x) {
	if (arena_wraps_at_edges) {
		if (x > Xmax)
			x = Xmin;
		if (x < Xmin)
			x = Xmax;
	} else {
		x = MIN(x, Xmax);
		x = MAX(x, Xmin);
	}
	return x;
}

double wrap_or_limit_y(double y) {
	if (arena_wraps_at_edges) {
		if (y > Ymax)
			y = Ymin;
		if (y < Ymin)
			y = Ymax;
	} else {
		y = MIN(y, Ymax);
		y = MAX(y, Ymin);
	}
	return y;
}


