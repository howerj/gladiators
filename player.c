#include "player.h"
#include "util.h"
#include "vars.h"
#include "gui.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>

void player_draw(player_t *p) {
	assert(p);
	if (!player_active)
		return;
	draw_regular_polygon_filled(p->x, p->y, p->orientation, p->radius - p->radius/2, TRIANGLE, WHITE);
	draw_regular_polygon_filled(p->x, p->y, p->orientation, p->radius, TRIANGLE, team_to_color(p->team));
	draw_line(p->x, p->y, p->orientation, p->radius*2, p->radius/2, MAGENTA);
}

player_t *player_new(unsigned team) {
	player_t *p = allocate(sizeof(*p));
	p->team = team;
	p->radius = player_size;
	return p;
}

void player_delete(player_t *p) {
	assert(p);
	free(p);
}

static void update_orientation(player_t *p, bool left, bool right) {
	assert(p);
	p->orientation  -= wrap_rad(right) / player_turn_rate_divisor;
	p->orientation  += wrap_rad(left)  / player_turn_rate_divisor;
	p->orientation   = wrap_rad(p->orientation);
}

static void update_distance(player_t *p, bool forward) {
	double distance = player_distance_per_tick * forward;
	distance = MAX(0, MIN(player_distance_per_tick, distance));
	p->x += distance * cos(p->orientation);
	p->x = wrap_or_limit_x(p->x);
	p->y += distance * sin(p->orientation);
	p->y = wrap_or_limit_y(p->y);
}

void player_update(player_t *p, bool fire, bool left, bool right, bool forward) {
	UNUSED(fire);
	assert(p);
	if (player_is_dead(p) || !player_active)
		return;
	if (p->energy < player_max_energy)
		p->energy += player_energy_increment;
	if (p->refire_timeout)
		p->refire_timeout--;
	update_orientation(p, left, right);
	update_distance(p, forward);
}

bool player_is_dead(player_t *p) {
	assert(p);
	return p->health < 0;
}

cell_t *player_serialize(player_t *p) {
	assert(p);
	cell_t *c = printer(
			"player "
			"(x %f) (y %f) (orientation %f) (health %f)"
			"(team %d) (hits %d) (foods %d)"
			"(energy %f) (score %f)",
			p->x, p->y, p->orientation, p->health,
			(intptr_t)(p->team), (intptr_t)(p->hits), (intptr_t)(p->foods),
			p->energy, p->score
			);
	assert(c);
	return c;
}

player_t *player_deserialize(cell_t *c) {
	assert(c);
	intptr_t team = 0, hits = 0, foods = 0;
	player_t *p = player_new(arena_gladiator_count);
	int r = scanner(c,
			"player "
			"(x %f) (y %f) (orientation %f) (health %f)"
			"(team %d) (hits %d) (foods %d)"
			"(energy %f) (score %f)",
			&p->x, &p->y, &p->orientation, &p->health,
			&team, &hits, &foods,
			&p->energy, &p->score);
	if (r < 0) {
		warning("deserialization into player object failed from cell <%p>", c);
		player_delete(p);
		return NULL;
	}
	p->team = team;
	p->hits = hits;
	p->foods = foods;

	return p;
}

