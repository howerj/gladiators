#include "food.h"
#include "vars.h"
#include "util.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>

food_t *food_new(double x, double y, double orientation)
{
	food_t *f = allocate(sizeof(*f));
	f->x = wrap_or_limit_x(x);
	f->y = wrap_or_limit_y(y);
	f->orientation = wrap_rad(orientation);
	f->radius = food_size;
	f->eaten = false;
	return f;
}

void food_delete(food_t *f)
{
	free(f);
}

void food_draw(food_t *f)
{
	draw_regular_polygon_line(f->x, f->y, 0, f->radius, SQUARE, 0.5, WHITE);
}

static inline double food_random(const double distance)
{
	return (random_float() * distance) - (distance / 2);
}

void food_update(food_t *f)
{
	const double distance = food_distance_per_tick;
	switch(food_control_method) {
	case FOOD_RANDOM_WALK_E:
		f->x += food_random(distance);
		f->y += food_random(distance);
		break;
	case FOOD_BOUNCE_E:
		f->x += distance * cos(f->orientation);
		f->x = wrap_or_limit_x(f->x);
		f->y += distance * sin(f->orientation);
		f->y = wrap_or_limit_y(f->y);
		if(f->y == Ymax || f->y == Ymin)
			f->orientation = wrap_rad(-(f->orientation));
		if(f->x == Xmax || f->x == Xmin)
			f->orientation = wrap_rad(-(f->orientation + PI));
		break;
	/*case 2: // avoid gladiators */
	default:
		error("invalid food control method: %u", food_control_method);
	}
}

bool food_is_active(food_t *f)
{
	return !f->eaten;
}

void food_reactivate(food_t *f, double x, double y, double orientation)
{
	f->eaten = false;
	f->x = wrap_or_limit_x(x);
	f->y = wrap_or_limit_y(y);
	f->orientation = wrap_rad(orientation);
}

void food_deactive(food_t *f)
{
	f->eaten = true;
}


