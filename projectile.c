#include "projectile.h"
#include "vars.h"
#include "util.h"
#include <math.h>

projectile_t *projectile_new(unsigned team, double x, double y, double orientation)
{
	projectile_t *p = allocate(sizeof(*p));
	p->x = wrap_or_limit_x(x);
	p->y = wrap_or_limit_y(y);
	p->orientation = orientation;
	p->team = team;
	p->travelled = projectile_range;
	p->radius = projectile_size;
	return p;
}

bool projectile_is_active(projectile_t *p)
{
	return p->travelled < projectile_range;
}

void projectile_remove(projectile_t *p)
{
	p->travelled += projectile_range;
}

void projectile_draw(projectile_t *p)
{
	if(!projectile_is_active(p))
		return;
	draw_regular_polygon_filled(p->x, p->y, p->orientation, projectile_size, TRIANGLE, team_to_color(p->team));
}

void projectile_update(projectile_t *p)
{
	const double distance = projectile_distance_per_tick;
	if(!projectile_is_active(p))
		return;
	p->x += distance * cos(p->orientation);
	p->x = wrap_or_limit_x(p->x);
	p->y += distance * sin(p->orientation);
	p->y = wrap_or_limit_y(p->y);
	p->travelled += distance;
	if(arena_wraps_at_edges == false && (p->x == Xmin || p->x == Xmax || p->y == Ymin || p->y == Ymax))
		projectile_remove(p);
}

bool projectile_fire(projectile_t *p, double x, double y, double orientation)
{
	if(projectile_is_active(p))
		return false;
	p->travelled = 0;
	p->x = wrap_or_limit_x(x);
	p->y = wrap_or_limit_y(y);
	p->orientation = orientation;
	return true;
}

