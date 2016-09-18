#ifndef GLADIATOR_H
#define GLADIATOR_H

#define GLADIATOR_SIZE              (3)
#define GLADIATOR_HEALTH            (2)
#define GLADIATOR_FIRE_TIMEOUT      (15)
#define GLADIATOR_BRAIN_LENGTH      (5)
#define GLADIATOR_DISTANCE_PER_TICK (0.5)
/**@note this could be added to a gladiators genetic code */
#define GLADIATOR_FIRE_THRESHOLD    (0.8)
#define GLADIATOR_VISION            (15)

#include <stdbool.h>
#include "util.h"
#include "brain.h"

typedef struct {
	double x, y;
	double orientation;
	double field_of_view;
	double health;
	double radius;
	unsigned team;
	unsigned hits;
	bool enemy_gladiator_detected;
	bool enemy_projectile_detected;
	brain_t *brain;
} gladiator_t;

typedef enum {
	GLADIATOR_IN_VISION_ENEMY,
	GLADIATOR_IN_VISION_PROJECTILE,
	GLADIATOR_IN_FIRED,
	GLADIATOR_IN_FIELD_OF_VIEW,
	GLADIATOR_IN_LAST_INPUT
} gladiator_input_e;

typedef enum {
	GLADIATOR_OUT_TURN_LEFT,
	GLADIATOR_OUT_TURN_RIGHT,
	GLADIATOR_OUT_MOVE_FORWARD,
	GLADIATOR_OUT_FIRE,
	GLADIATOR_OUT_FIELD_OF_VIEW,
	GLADIATOR_OUT_LAST_OUTPUT
} gladiator_output_e;

void draw_gladiator(gladiator_t *g);
gladiator_t *new_gladiator(prng_t *p, unsigned team, double x, double y, double orientation);
void update_gladiator(gladiator_t *g, const double inputs[], double outputs[]);
void delete_gladiator(gladiator_t *g);

#endif
