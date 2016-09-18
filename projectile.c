#include "projectile.h"
#include "util.h"
#include <math.h>

projectile_t *projectile_new(unsigned team, double x, double y, double orientation)
{
	projectile_t *p = allocate(sizeof(*p));
	p->x = wrapx(x);
	p->y = wrapy(y);
	p->orientation = orientation;
	p->team = team;
	p->travelled = PROJECTILE_RANGE;
	p->radius = PROJECTILE_SIZE;
	return p;
}

bool projectile_is_active(projectile_t *p)
{
	return p->travelled < PROJECTILE_RANGE;
}

void projectile_draw(projectile_t *p)
{
	if(!projectile_is_active(p))
		return;
	draw_regular_polygon_filled(p->x, p->y, p->orientation, PROJECTILE_SIZE, TRIANGLE, team_to_color(p->team));
}

void projectile_update(projectile_t *p)
{
	const double distance = PROJECTILE_DISTANCE_PER_TICK;
	if(!projectile_is_active(p))
		return;
	p->x += distance * cos(p->orientation);
	p->x = wrapx(p->x);
	p->y += distance * sin(p->orientation);
	p->y = wrapy(p->y);
	p->travelled += distance;
}

bool projectile_fire(projectile_t *p, double x, double y, double orientation)
{
	if(projectile_is_active(p))
		return false;
	p->travelled = 0;
	p->x = wrapx(x);
	p->y = wrapy(y);
	p->orientation = orientation;
	return true;
}


