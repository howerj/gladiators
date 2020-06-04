/** @file       food.c
 *  @brief      Food object
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/
#include "food.h"
#include "vars.h"
#include "util.h"
#include "gui.h"
#include "wrap.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

food_t *food_new(double x, double y, double orientation) {
	food_t *f = allocate(sizeof(*f));
	f->x = wrap_or_limit_x(x);
	f->y = wrap_or_limit_y(y);
	f->orientation = wrap_rad(orientation);
	f->radius = food_size;
	f->eaten = false;
	return f;
}

void food_delete(food_t *f) {
	free(f);
}

void food_draw(food_t *f) {
	assert(f);
	draw_regular_polygon_line(f->x, f->y, 0, f->radius, SQUARE, 0.5, WHITE);
}

static inline double food_random(const double distance) {
	return (random_float() * distance) - (distance / 2);
}

void food_update(food_t *f) {
	const double distance = food_distance_per_tick;
	switch (food_control_method) {
	case FOOD_RANDOM_WALK_E:
		f->x += food_random(distance);
		f->y += food_random(distance);
		break;
	case FOOD_BOUNCE_E:
		f->x += distance * cos(f->orientation);
		f->x = wrap_or_limit_x(f->x);
		f->y += distance * sin(f->orientation);
		f->y = wrap_or_limit_y(f->y);
		if (f->y == Ymax || f->y == Ymin)
			f->orientation = wrap_rad(-(f->orientation));
		if (f->x == Xmax || f->x == Xmin)
			f->orientation = wrap_rad(-(f->orientation + PI));
		break;
	/*case 2: // avoid gladiators */
	default:
		error("invalid food control method: %u", food_control_method);
	}
}

bool food_is_active(food_t *f) {
	assert(f);
	return !f->eaten;
}

void food_reactivate(food_t *f, double x, double y, double orientation) {
	assert(f);
	f->eaten = false;
	f->x = wrap_or_limit_x(x);
	f->y = wrap_or_limit_y(y);
	f->orientation = wrap_rad(orientation);
}

void food_deactive(food_t *f) {
	assert(f);
	f->eaten = true;
}

cell_t *food_serialize(food_t *f) {
	assert(f);
	cell_t *n = printer("food (x %f) (y %f) (orientation %f) (eaten %d)",
			f->x, f->y, f->orientation, (intptr_t)(f->eaten));
	assert(n);
	return n;
}

food_t *food_deserialize(cell_t *c) {
	assert(c);
	food_t *f = food_new(0, 0, 0);
	intptr_t eaten = false;
	int r = scanner(c, "food (x %f) (y %f) (orientation %f) (eaten %d)",
			&f->x, &f->y, &f->orientation, &f->eaten);
	f->eaten = eaten;
	if (r < 0) {
		warning("could not deserialize food object <%p>", c);
		food_delete(f);
		return NULL;
	}
	return f;
}

