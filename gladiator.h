/** @file       gladiator.h
 *  @brief      gladiator object, these gladiators battle and evolve
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

#ifndef GLADIATOR_H
#define GLADIATOR_H

#include <stdbool.h>
#include "brain.h"
#include "util.h"
#include "color.h"

typedef struct {
	double x, y; /**< position of gladiator*/
	double orientation; /**< direction of gladiator*/
	double field_of_view; /**< angle of gladiators eye*/
	double health; /**< health of this gladiator*/
	double radius; /**< size of this gladiator*/
	unsigned team; /**< this gladiators team*/
	unsigned hits; /**< hits the gladiator scored on other gladiators*/
	unsigned foods; /**< number of food items collected*/
	unsigned round; /**< current round gladiator is in*/
	double energy; /**< energy of gladiator, needed to fire weapon*/
	bool enemy_gladiator_detected; /**< set true if enemy has been detected*/
	bool enemy_projectile_detected; /**< set true if enemy projectile has been detected*/
	bool food_detected; /**< set true if food has been detected */
	double state1; /**< internal state, experimental*/
	unsigned time_alive; /**< when the gladiator died, if it did*/
	unsigned mutations; /**< mutations from previous round*/
	unsigned total_mutations; /**< total mutations*/
	unsigned refire_timeout; /**< time left until next fire allowed */
	double fitness; /**< parents fitness level*/
	brain_t *brain; /**< the gladiators brain*/
	timer_tick_t wall_contact_timer; /**< timer for the amount of gladiator has been in contact with the wall*/
	color_t color;
} gladiator_t;

#define X_MACRO_GLADIATOR_INPUTS\
	X(GLADIATOR_IN_VISION_ENEMY,       "detect hostile gladiator")\
	X(GLADIATOR_IN_VISION_PROJECTILE,  "detect hostile projectile")\
	X(GLADIATOR_IN_VISION_FOOD,        "detect food")\
	X(GLADIATOR_IN_CAN_FIRE,           "gladiator weapon ready")\
	X(GLADIATOR_IN_HIT_GLADIATOR,      "gladiator hit enermy")\
	X(GLADIATOR_IN_FIELD_OF_VIEW,      "gladiator field of view angle")\
	X(GLADIATOR_IN_COLLISION_WALL,     "wall collision")\
	X(GLADIATOR_IN_COLLISION_ENEMY,    "hostile gladiator collision")\
	X(GLADIATOR_IN_STATE1,             "gladiator memory")\
	X(GLADIATOR_IN_X,                  "gladiator x position")\
	X(GLADIATOR_IN_Y,                  "gladiator y position")\
	X(GLADIATOR_IN_ORIENTATION,        "gladiator angle")\
	X(GLADIATOR_IN_RANDOM,             "gladiator entropy source")\
	X(GLADIATOR_IN_LAST_INPUT,         "INVALID INPUT")

typedef enum {
#define X(ENUM, DESCRIPTION) ENUM,
	X_MACRO_GLADIATOR_INPUTS
#undef X
} gladiator_input_e;

#define X_MACRO_GLADIATOR_OUTPUTS\
	X(GLADIATOR_OUT_TURN_LEFT,            "left turn")\
	X(GLADIATOR_OUT_TURN_RIGHT,           "right turn")\
	X(GLADIATOR_OUT_MOVE_FORWARD,         "forward")\
	X(GLADIATOR_OUT_FIRE,                 "fire")\
	X(GLADIATOR_OUT_FIELD_OF_VIEW_OPEN,   "open field of view")\
	X(GLADIATOR_OUT_FIELD_OF_VIEW_CLOSE,  "close field of view")\
	X(GLADIATOR_OUT_STATE1,               "gladiator memory")\
	X(GLADIATOR_OUT_LAST_OUTPUT,          "INVALID OUTPUT")

typedef enum {
#define X(ENUM, DESCRIPTION) ENUM,
	X_MACRO_GLADIATOR_OUTPUTS
#undef X
} gladiator_output_e;

void gladiator_draw(gladiator_t *g);
gladiator_t *gladiator_new(unsigned team, double x, double y, double orientation);
gladiator_t *gladiator_copy(gladiator_t *g);
void gladiator_update(gladiator_t *g, const double inputs[], double outputs[]);
void gladiator_delete(gladiator_t *g);
double gladiator_fitness(gladiator_t *g);
unsigned gladiator_mutate(gladiator_t *g);
bool gladiator_is_dead(gladiator_t *g);
const char *lookup_gladiator_io_name(bool lookup_input, unsigned port);
gladiator_t *gladiator_breed(gladiator_t *a, gladiator_t *b);
cell_t *gladiator_serialize(gladiator_t *p);
gladiator_t *gladiator_deserialize(cell_t *c);

#endif
