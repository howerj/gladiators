#include "gladiator.h"
#include "brain.h"
#include "util.h"
#include "color.h"
#include "vars.h"
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

static void update_field_of_view(gladiator_t *g, double outputs[])
{
	g->field_of_view += outputs[GLADIATOR_OUT_FIELD_OF_VIEW_OPEN] / gladiator_field_of_view_divisor;
	g->field_of_view -= outputs[GLADIATOR_OUT_FIELD_OF_VIEW_CLOSE] / gladiator_field_of_view_divisor;
	g->field_of_view = MIN(g->field_of_view, gladiator_max_field_of_view);
	g->field_of_view = MAX(g->field_of_view, gladiator_min_field_of_view);
}

static void update_orientation(gladiator_t *g, double outputs[])
{
	g->orientation  += wraprad(outputs[GLADIATOR_OUT_TURN_LEFT]) / gladiator_turn_rate_divisor; 
	g->orientation  -= wraprad(outputs[GLADIATOR_OUT_TURN_RIGHT]) / gladiator_turn_rate_divisor;
	g->orientation   = wraprad(g->orientation);
}

static void update_distance(gladiator_t *g, double outputs[])
{
	/**@todo add inertia */
	const double distance = gladiator_distance_per_tick * outputs[GLADIATOR_OUT_MOVE_FORWARD];
	g->x += distance * cos(g->orientation);
	g->x = wrap_or_limit_x(g->x);
	g->y += distance * sin(g->orientation);
	g->y = wrap_or_limit_y(g->y);
}

bool gladiator_is_dead(gladiator_t *g)
{
	return g->health < 0;
}

void gladiator_update(gladiator_t *g, const double inputs[], double outputs[])
{
	if(gladiator_is_dead(g))
		return;
	if(g->energy < gladiator_max_energy)
		g->energy += gladiator_energy_increment;
	g->enemy_gladiator_detected  = inputs[GLADIATOR_IN_VISION_ENEMY] > 0.0;
	g->enemy_projectile_detected = inputs[GLADIATOR_IN_VISION_PROJECTILE] > 0.0;

	brain_update(g->brain, inputs, GLADIATOR_IN_LAST_INPUT, outputs, GLADIATOR_OUT_LAST_OUTPUT);

	g->state1 = outputs[GLADIATOR_IN_STATE1];
	update_field_of_view(g, outputs);
	update_orientation(g, outputs);
	update_distance(g, outputs);
}

void gladiator_draw(gladiator_t *g)
{
	color_t projectile = g->enemy_projectile_detected ? RED : GREEN;
	draw_line(g->x, g->y, g->orientation, g->radius*2, g->radius/2, projectile);
	if(!gladiator_is_dead(g)) {
		color_t target = g->enemy_gladiator_detected ? RED : GREEN;
		draw_line(g->x, g->y, g->orientation - g->field_of_view/2, Ymax/5, g->radius/2, target);
		draw_line(g->x, g->y, g->orientation + g->field_of_view/2, Ymax/5, g->radius/2, target);
	}
	draw_regular_polygon_filled(g->x, g->y, g->orientation, g->radius/2, CIRCLE, !gladiator_is_dead(g) ? WHITE : BLACK);
	draw_regular_polygon_filled(g->x, g->y, g->orientation, g->radius, PENTAGON, team_to_color(g->team));
}

gladiator_t *gladiator_new(unsigned team, double x, double y, double orientation)
{
	gladiator_t *g = allocate(sizeof(*g));
	g->team = team;
	g->x = wrap_or_limit_x(x);
	g->y = wrap_or_limit_y(y);
	g->orientation = orientation;
	g->field_of_view = PI / 3.0;
	g->health = gladiator_health;
	g->radius = gladiator_size;
	size_t length = MAX(gladiator_brain_length, GLADIATOR_IN_LAST_INPUT);
	length = MAX(length, GLADIATOR_OUT_LAST_OUTPUT);
	g->brain = brain_new(true, length, gladiator_brain_depth);
	return g;
}

void gladiator_delete(gladiator_t *g)
{
	brain_delete(g->brain);
	free(g);
}

unsigned gladiator_mutate(gladiator_t *g)
{
	return brain_mutate(g->brain);
}

gladiator_t *gladiator_copy(gladiator_t *g)
{
	gladiator_t *n = gladiator_new(g->team, g->x, g->y, g->orientation);
	brain_delete(n->brain);
	n->state1 = g->state1;
	n->total_mutations = g->total_mutations;
	n->previous_fitness = g->previous_fitness;
	n->brain = brain_copy(g->brain);
	return n;
}

double gladiator_fitness(gladiator_t *g)
{
	double fitness = 0.0;
	fitness += g->health * fitness_weight_health;
	fitness += g->hits   * fitness_weight_hits;
	fitness += g->energy * fitness_weight_energy;
	fitness += ((arena_gladiator_count / (g->rank + 1)) - 1) * fitness_weight_rank;
	return fitness;
}

