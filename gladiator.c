#include "gladiator.h"
#include "brain.h"
#include "util.h"
#include "color.h"
#include "team.h"
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

void update_gladiator(gladiator_t *g, const double inputs[], double outputs[])
{
	if(g->health < 0)
		return;
	g->enemy_gladiator_detected  = inputs[GLADIATOR_IN_VISION_ENEMY] > 0.0;
	g->enemy_projectile_detected = inputs[GLADIATOR_IN_VISION_PROJECTILE] > 0.0;
	update_brain(g->brain, inputs, GLADIATOR_IN_LAST_INPUT, outputs, GLADIATOR_OUT_LAST_OUTPUT);
	g->field_of_view = outputs[GLADIATOR_OUT_FIELD_OF_VIEW];
	g->orientation  += outputs[GLADIATOR_OUT_TURN_LEFT];
	g->orientation  -= outputs[GLADIATOR_OUT_TURN_RIGHT];
	g->orientation   = wraprad(g->orientation);
	const double distance = GLADIATOR_DISTANCE_PER_TICK * outputs[GLADIATOR_OUT_MOVE_FORWARD];
	g->x += distance * cos(g->orientation);
	g->x = wrapx(g->x);
	g->y += distance * sin(g->orientation);
	g->y = wrapy(g->y);
}

void draw_gladiator(gladiator_t *g)
{
	color_t projectile = g->enemy_projectile_detected ? RED : GREEN;
	draw_line(g->x, g->y, g->orientation, g->radius*2, g->radius/2, projectile);
	if(g->health >= 0) {
		color_t target = g->enemy_gladiator_detected ? RED : GREEN;
		draw_line(g->x, g->y, g->orientation - g->field_of_view/2, g->radius*GLADIATOR_VISION, g->radius/2, target);
		draw_line(g->x, g->y, g->orientation + g->field_of_view/2, g->radius*GLADIATOR_VISION, g->radius/2, target);
	}
	draw_regular_polygon(g->x, g->y, g->orientation, g->radius/2, CIRCLE, g->health > 0 ? WHITE : BLACK);
	draw_regular_polygon(g->x, g->y, g->orientation, g->radius, PENTAGON, team_to_color(g->team));
}

gladiator_t *new_gladiator(prng_t *p, unsigned team, double x, double y, double orientation)
{
	gladiator_t *g = allocate(sizeof(*g));
	g->team = team;
	g->x = wrapx(x);
	g->y = wrapy(y);
	g->orientation = orientation;
	g->field_of_view = PI / 3;
	g->health = GLADIATOR_HEALTH;
	g->radius = GLADIATOR_SIZE;
	g->brain = new_brain(p, GLADIATOR_BRAIN_LENGTH);
	return g;
}

void delete_gladiator(gladiator_t *g)
{
	delete_brain(g->brain);
	free(g);
}

