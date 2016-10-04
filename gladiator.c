/** @file       gladiator.c
 *  @brief      a "gladiator" - an entity we are trying to improve using
 *              genetic algorithms
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

#include "gladiator.h"
#include "brain.h"
#include "util.h"
#include "color.h"
#include "vars.h"
#include "gui.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

static const char *gladiator_input_names[] = {
#define X(ENUM, DESCRIPTION) DESCRIPTION,
	X_MACRO_GLADIATOR_INPUTS
#undef X
};

static const char *gladiator_output_names[] = {
#define X(ENUM, DESCRIPTION) DESCRIPTION,
	X_MACRO_GLADIATOR_INPUTS
#undef X
};

const char *lookup_gladiator_io_name(bool lookup_input, unsigned port)
{
	if(lookup_input) {
		if(port >= GLADIATOR_IN_LAST_INPUT)
			error("'%u' is not a valid gladiator input port");
		return gladiator_input_names[port];
	} else {
		if(port >= GLADIATOR_OUT_LAST_OUTPUT)
			error("'%u' is not a valid gladiator output port");
		return gladiator_output_names[port];
	}
	return NULL;
}


static void update_field_of_view(gladiator_t *g, double outputs[])
{
	assert(g && outputs);
	g->field_of_view += outputs[GLADIATOR_OUT_FIELD_OF_VIEW_OPEN] / gladiator_field_of_view_divisor;
	g->field_of_view -= outputs[GLADIATOR_OUT_FIELD_OF_VIEW_CLOSE] / gladiator_field_of_view_divisor;
	g->field_of_view = MIN(g->field_of_view, gladiator_max_field_of_view);
	g->field_of_view = MAX(g->field_of_view, gladiator_min_field_of_view);
}

static void update_orientation(gladiator_t *g, double outputs[])
{
	assert(g && outputs);
	g->orientation  += wrap_rad(outputs[GLADIATOR_OUT_TURN_LEFT]) / gladiator_turn_rate_divisor; 
	g->orientation  -= wrap_rad(outputs[GLADIATOR_OUT_TURN_RIGHT]) / gladiator_turn_rate_divisor;
	g->orientation   = wrap_rad(g->orientation);
}

static void update_distance(gladiator_t *g, double outputs[])
{
	assert(g && outputs);
	/**@todo add inertia, drag could also be added, a value that could be
	 * changed to reflect traveling in water, or air. The physics section
	 * and function really should be separated into another file. */
	double distance = gladiator_distance_per_tick * outputs[GLADIATOR_OUT_MOVE_FORWARD];
	distance = MAX(0, MIN(gladiator_distance_per_tick, distance));
	g->x += distance * cos(g->orientation);
	g->x = wrap_or_limit_x(g->x);
	g->y += distance * sin(g->orientation);
	g->y = wrap_or_limit_y(g->y);

	if(gladiator_bounce_off_walls) {
		if(g->y == Ymax || g->y == Ymin)
			g->orientation = wrap_rad(-(g->orientation));
		if(g->x == Xmax || g->x == Xmin)
			g->orientation = wrap_rad(-(g->orientation + PI));
	}
}

bool gladiator_is_dead(gladiator_t *g)
{
	assert(g);
	return g->health < 0;
}

void gladiator_update(gladiator_t *g, const double inputs[], double outputs[])
{
	assert(g);
	if(gladiator_is_dead(g))
		return;
	if(g->energy < gladiator_max_energy)
		g->energy += gladiator_energy_increment;
	g->enemy_gladiator_detected  = inputs[GLADIATOR_IN_VISION_ENEMY] > 0.0;
	g->enemy_projectile_detected = inputs[GLADIATOR_IN_VISION_PROJECTILE] > 0.0;
	g->food_detected = inputs[GLADIATOR_IN_VISION_FOOD] > 0.0;

	brain_update(g->brain, inputs, GLADIATOR_IN_LAST_INPUT, outputs, GLADIATOR_OUT_LAST_OUTPUT);

	g->state1 = outputs[GLADIATOR_IN_STATE1];
	update_field_of_view(g, outputs);
	update_orientation(g, outputs);
	update_distance(g, outputs);
	if(arena_wraps_at_edges == false && (g->x == Xmax || g->x == Xmin || g->y == Ymax || g->y == Ymin))
		timer_tick(&g->wall_contact_timer);
	else if(!timer_result(&g->wall_contact_timer))
		timer_untick(&g->wall_contact_timer);
}

void gladiator_draw(gladiator_t *g)
{
	assert(g);
	/**@todo draw gladiator inputs as "eyes" on the gladiator*/
	color_t food = g->food_detected ? BLUE : GREEN;
	draw_regular_polygon_filled(g->x, g->y, g->orientation, g->radius/4, CIRCLE, food);
	draw_regular_polygon_filled(g->x, g->y, g->orientation, g->radius/2, CIRCLE, g->health > 0 ? WHITE : BLACK);
	draw_regular_polygon_filled(g->x, g->y, g->orientation, g->radius, PENTAGON, team_to_color(g->team));

	color_t projectile = g->enemy_projectile_detected ? RED : GREEN;
	draw_line(g->x, g->y, g->orientation, g->radius*2, g->radius/2, projectile);
	if(!gladiator_is_dead(g) && draw_gladiator_target_lines) {
		color_t target = g->enemy_gladiator_detected ? RED : GREEN;
		draw_line(g->x, g->y, g->orientation - g->field_of_view/2, Ymax/5, g->radius/2, target);
		draw_line(g->x, g->y, g->orientation + g->field_of_view/2, Ymax/5, g->radius/2, target);
	}
}

gladiator_t *gladiator_new(unsigned team, double x, double y, double orientation)
{
	assert(team < arena_gladiator_count);
	assert(x >= Xmin && x <= Xmax);
	assert(y >= Ymin && y <= Ymax);
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
	g->brain = brain_new(true, true, length, gladiator_brain_depth);
	return g;
}

void gladiator_delete(gladiator_t *g)
{
	assert(g);
	brain_delete(g->brain);
	free(g);
}

unsigned gladiator_mutate(gladiator_t *g)
{
	assert(g);
	return brain_mutate(g->brain);
}

gladiator_t *gladiator_copy(gladiator_t *g)
{
	assert(g);
	gladiator_t *n = gladiator_new(g->team, g->x, g->y, g->orientation);
	brain_delete(n->brain);
	n->state1 = g->state1;
	n->fitness = g->fitness;
	n->total_mutations = g->total_mutations;
	n->brain = brain_copy(g->brain);
	return n;
}

double gladiator_fitness(gladiator_t *g)
{
	assert(g);
	double fitness = 0.0;
	fitness += g->health * fitness_weight_health;
	fitness += g->hits   * fitness_weight_hits;
	fitness += g->energy * fitness_weight_energy;
	fitness += g->foods  * fitness_weight_food;
	fitness += g->rank   * fitness_weight_rank;
	fitness += timer_result(&g->wall_contact_timer) * fitness_weight_wall_time;
	return fitness;
}

gladiator_t *gladiator_breed(gladiator_t *a, gladiator_t *b)
{
	gladiator_t *child = gladiator_new(a->team, 0, 0, 0);
	brain_delete(child->brain);
	child->mutations = MAX(a->mutations, b->mutations);
	child->rank      = MAX(a->rank,      b->rank);
	child->fitness   = MAX(a->fitness,   b->fitness);
	child->brain = brain_crossover(a->brain, b->brain);
	return child;
}

