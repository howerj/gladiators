#ifndef GLADIATOR_H
#define GLADIATOR_H

#include <stdbool.h>
#include "brain.h"

typedef struct {
	double x, y; /*position of gladiator*/
	double orientation; /*direction of gladiator*/
	double field_of_view; /*angle of gladiators eye*/
	double health; /*health of this gladiator*/
	double radius; /*size of this gladiator*/
	unsigned team; /*this gladiators team*/
	unsigned hits; /*hits the gladiator scored on other gladiators*/
	double energy; /*energy of gladiator, needed to fire weapon*/
	bool enemy_gladiator_detected;
	bool enemy_projectile_detected;
	double state1; /*internal state, experimental*/
	unsigned rank; /*when the gladiator died, if it did*/
	unsigned mutations; /*mutations from previous round*/
	unsigned total_mutations; /*total mutations*/
	double previous_fitness; /*parents fitness level*/
	brain_t *brain;
} gladiator_t;

typedef enum {
	GLADIATOR_IN_VISION_ENEMY,
	GLADIATOR_IN_VISION_PROJECTILE,
	GLADIATOR_IN_CAN_FIRE,
	GLADIATOR_IN_FIELD_OF_VIEW,
	GLADIATOR_IN_COLLISION_WALL,
	GLADIATOR_IN_COLLISION_ENEMY,
	GLADIATOR_IN_STATE1,
	GLADIATOR_IN_RANDOM,
	GLADIATOR_IN_LAST_INPUT
} gladiator_input_e;

typedef enum {
	GLADIATOR_OUT_TURN_LEFT,
	GLADIATOR_OUT_TURN_RIGHT,
	GLADIATOR_OUT_MOVE_FORWARD,
	GLADIATOR_OUT_FIRE,
	GLADIATOR_OUT_FIELD_OF_VIEW_OPEN,
	GLADIATOR_OUT_FIELD_OF_VIEW_CLOSE,
	GLADIATOR_OUT_STATE1,
	GLADIATOR_OUT_LAST_OUTPUT
} gladiator_output_e;

void gladiator_draw(gladiator_t *g);
gladiator_t *gladiator_new(unsigned team, double x, double y, double orientation);
gladiator_t *gladiator_copy(gladiator_t *g);
void gladiator_update(gladiator_t *g, const double inputs[], double outputs[]);
void gladiator_delete(gladiator_t *g);
double gladiator_fitness(gladiator_t *g);
unsigned gladiator_mutate(gladiator_t *g);
bool gladiator_is_dead(gladiator_t *g);

#endif
