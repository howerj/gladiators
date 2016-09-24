/** @file       projectile.c
 *  @brief      
 *  @author     Richard Howe (2016)
 *  @license    LGPL v2.1 or Later 
 *              <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html> 
 *  @email      howe.r.j.89@gmail.com*/

#include "projectile.h"
#include "vars.h"
#include "util.h"
#include <assert.h>
#include <math.h>

projectile_t *projectile_new(unsigned team, double x, double y, double orientation)
{
	assert(team < arena_gladiator_count);
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

bool projectile_is_active(projectile_t *p)
{
	assert(p);
	return p->travelled < projectile_range;
}

void projectile_deactivate(projectile_t *p)
{
	assert(p);
	p->travelled += projectile_range;
}

void projectile_draw(projectile_t *p)
{
	assert(p);
	if(!projectile_is_active(p))
		return;
	draw_regular_polygon_filled(p->x, p->y, p->orientation, projectile_size, TRIANGLE, team_to_color(p->team));
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

bool projectile_fire(projectile_t *p, double x, double y, double orientation)
{
	assert(p);
	if(projectile_is_active(p))
		return false;
	p->travelled = 0;
	p->x = wrap_or_limit_x(x);
	p->y = wrap_or_limit_y(y);
	p->orientation = orientation;
	return true;
}

