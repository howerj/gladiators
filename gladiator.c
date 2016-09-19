#include "gladiator.h"
#include "brain.h"
#include "util.h"
#include "color.h"
#include "vars.h"
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

void gladiator_update(gladiator_t *g, const double inputs[], double outputs[])
{
	if(g->health < 0)
		return;
	g->energy++;
	g->enemy_gladiator_detected  = inputs[GLADIATOR_IN_VISION_ENEMY] > 0.0;
	g->enemy_projectile_detected = inputs[GLADIATOR_IN_VISION_PROJECTILE] > 0.0;
	brain_update(g->brain, inputs, GLADIATOR_IN_LAST_INPUT, outputs, GLADIATOR_OUT_LAST_OUTPUT);
	g->field_of_view = outputs[GLADIATOR_OUT_FIELD_OF_VIEW];
	g->orientation  += wraprad(outputs[GLADIATOR_OUT_TURN_LEFT])/12; /**@todo make configurable*/
	g->orientation  -= wraprad(outputs[GLADIATOR_OUT_TURN_RIGHT])/12;
	g->orientation   = wraprad(g->orientation);
	/**@todo add inertia */
	const double distance = gladiator_distance_per_tick * outputs[GLADIATOR_OUT_MOVE_FORWARD] / brain_max_weight;
	g->x += distance * cos(g->orientation);
	g->x = wrapx(g->x);
	g->y += distance * sin(g->orientation);
	g->y = wrapy(g->y);
}

void gladiator_draw(gladiator_t *g)
{
	color_t projectile = g->enemy_projectile_detected ? RED : GREEN;
	draw_line(g->x, g->y, g->orientation, g->radius*2, g->radius/2, projectile);
	if(g->health >= 0) {
		color_t target = g->enemy_gladiator_detected ? RED : GREEN;
		draw_line(g->x, g->y, g->orientation - g->field_of_view/2, Ymax/5, g->radius/2, target);
		draw_line(g->x, g->y, g->orientation + g->field_of_view/2, Ymax/5, g->radius/2, target);
	}
	draw_regular_polygon_filled(g->x, g->y, g->orientation, g->radius/2, CIRCLE, g->health > 0 ? WHITE : BLACK);
	draw_regular_polygon_filled(g->x, g->y, g->orientation, g->radius, PENTAGON, team_to_color(g->team));
}

gladiator_t *gladiator_new(unsigned team, double x, double y, double orientation)
{
	gladiator_t *g = allocate(sizeof(*g));
	g->team = team;
	g->x = wrapx(x);
	g->y = wrapy(y);
	g->orientation = orientation;
	g->field_of_view = PI / 3;
	g->health = gladiator_health;
	g->radius = gladiator_size;
	g->brain = brain_new(true, gladiator_brain_length);
	return g;
}

void gladiator_delete(gladiator_t *g)
{
	brain_delete(g->brain);
	free(g);
}


void gladiator_mutate(gladiator_t *g)
{
	brain_mutate(g->brain);
}

gladiator_t *gladiator_copy(gladiator_t *g)
{
	gladiator_t *n = gladiator_new(g->team, g->x, g->y, g->orientation);
	brain_delete(n->brain);
	n->brain = brain_copy(g->brain);
	return n;
}

double gladiator_fitness(gladiator_t *g)
{
	/**@todo think of a better fitness function*/
	return g->health * 1.5 + g->hits;
}

