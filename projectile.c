/** @file       projectile.c
 *  @brief      Projectile object, gladiators can fire these
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

#include "projectile.h"
#include "vars.h"
#include "util.h"
#include "gui.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

projectile_t *projectile_new(unsigned team, double x, double y, double orientation)
{
	assert(team < arena_gladiator_count || team == (unsigned)-1l);
	assert(x >= Xmin && x <= Xmax);
	assert(y >= Ymin && y <= Ymax);
	projectile_t *p = allocate(sizeof(*p));
	p->x = wrap_or_limit_x(x);
	p->y = wrap_or_limit_y(y);
	p->orientation = orientation;
	p->team = team;
	p->travelled = projectile_range;
	p->radius = projectile_size;
	return p;
}

void projectile_delete(projectile_t *p)
{
	free(p);
}

bool projectile_is_active(projectile_t *p)
{
	assert(p);
	return p->travelled < projectile_range;
}

void projectile_deactivate(projectile_t *p)
{
	assert(p);
	p->travelled += projectile_range;
	p->team = (unsigned)-1l;
}

void projectile_draw(projectile_t *p)
{
	assert(p);
	if(!projectile_is_active(p))
		return;
	const color_t *color = p->color ? p->color : RED;
	draw_regular_polygon_filled(p->x, p->y, p->orientation, projectile_size, TRIANGLE, color);
}

unsigned projectile_team(projectile_t *p)
{
	assert(p);
	return p->team;
}

void projectile_update(projectile_t *p)
{
	assert(p);
	if(!projectile_is_active(p))
		return;

	const double distance = projectile_distance_per_tick;
	p->x += distance * cos(p->orientation);
	p->x = wrap_or_limit_x(p->x);
	p->y += distance * sin(p->orientation);
	p->y = wrap_or_limit_y(p->y);
	p->travelled += distance;
	if(arena_wraps_at_edges == false && (p->x == Xmin || p->x == Xmax || p->y == Ymin || p->y == Ymax))
		projectile_deactivate(p);
}

bool projectile_fire(projectile_t *p, unsigned team, double x, double y, double orientation, const color_t *color)
{
	assert(p);
	if(projectile_is_active(p))
		return false;
	p->color = color;
	p->travelled = 0;
	p->x = wrap_or_limit_x(x);
	p->y = wrap_or_limit_y(y);
	p->orientation = orientation;
	p->team = team;
	return true;
}

cell_t *projectile_serialize(projectile_t *p)
{
	assert(p);
	cell_t *c = printer("projectile (team %d) (x %f) (y %f) (orientation %f) (travelled %f)",
			p->team,
			p->x,
			p->y,
			p->orientation);
	assert(c);
	return c;
}

projectile_t *projectile_deserialize(cell_t *c)
{
	assert(c);
	projectile_t *p = projectile_new(0, 0, 0, 0);
	int r = scanner(c, "projectile (team %d) (x %f) (y %f) (orientation %f) (travelled %f)", 
			&p->team, &p->x, &p->y, &p->orientation, &p->travelled);

	if(r < 0) {
		projectile_delete(p);
		return NULL;
	}
	return p;
}


