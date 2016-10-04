
#include "player.h"
#include "util.h"
#include "vars.h"
#include "gui.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>

void player_draw(player_t *p)
{
	assert(p);
	if(!player_active)
		return;
	draw_regular_polygon_filled(p->x, p->y, p->orientation, p->radius - p->radius/2, TRIANGLE, WHITE);
	draw_regular_polygon_filled(p->x, p->y, p->orientation, p->radius, TRIANGLE, team_to_color(p->team));
	draw_line(p->x, p->y, p->orientation, p->radius*2, p->radius/2, MAGENTA);
}

player_t *player_new(unsigned team)
{
	player_t *p = allocate(sizeof(*p));
	p->team = team;
	p->radius = player_size;
	return p;
}

void player_delete(player_t *p)
{
	assert(p);
	free(p);
}

static void update_orientation(player_t *p, bool left, bool right)
{
	assert(p);
	p->orientation  -= wrap_rad(right);
	p->orientation  += wrap_rad(left);
	p->orientation   = wrap_rad(p->orientation);
}

void update_distance(player_t *p, bool forward)
{
	double distance = player_distance_per_tick * forward;
	distance = MAX(0, MIN(player_distance_per_tick, distance));
	p->x += distance * cos(p->orientation);
	p->x = wrap_or_limit_x(p->x);
	p->y += distance * sin(p->orientation);
	p->y = wrap_or_limit_y(p->y);
}

void player_update(player_t *p, bool fire, bool left, bool right, bool forward)
{
	UNUSED(fire);
	assert(p);
	if(player_is_dead(p) || !player_active)
		return;
	update_orientation(p, left, right);
	update_distance(p, forward);
}

bool player_is_dead(player_t *p)
{
	assert(p);
	return p->health < 0;
}

