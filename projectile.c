#include "projectile.h"
#include "util.h"
#include "team.h"
#include <math.h>

projectile_t *new_projectile(unsigned team, double x, double y, double orientation)
{
	projectile_t *p = allocate(sizeof(*p));
	p->x = wrapx(x);
	p->y = wrapy(y);
	p->orientation = orientation;
	p->team = team;
	p->travelled = PROJECTILE_RANGE;
	return p;
}

bool is_projectile_active(projectile_t *p)
{
	return p->travelled < PROJECTILE_RANGE;
}

void draw_projectile(projectile_t *p)
{
	if(!is_projectile_active(p))
		return;
	draw_regular_polygon(p->x, p->y, p->orientation, PROJECTILE_SIZE, TRIANGLE, team_to_color(p->team));
}

void update_projectile(projectile_t *p)
{
	const double distance = PROJECTILE_DISTANCE_PER_TICK;
	if(!is_projectile_active(p))
		return;
	p->x += distance * cos(p->orientation);
	p->x = wrapx(p->x);
	p->y += distance * sin(p->orientation);
	p->y = wrapy(p->y);
	p->travelled += distance;
}

bool fire_projectile(projectile_t *p, double x, double y, double orientation)
{
	if(is_projectile_active(p))
		return false;
	p->travelled = 0;
	p->x = wrapx(x);
	p->y = wrapy(y);
	p->orientation = orientation;
	return true;
}


