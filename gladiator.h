#ifndef GLADIATOR_H
#define GLADIATOR_H

#define GLADIATOR_SIZE  (3)

struct brain_t;
typedef struct brain_t brain_t;

typedef struct {
	double x, y;
	double orientation;
	double field_of_view;
	double health;
	unsigned team;
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
	GLADIATOR_OUT_CHANGE_FIELD_OF_VIEW,
	GLADIATOR_OUT_LAST_INPUT
} gladiator_output_e;

void draw_gladiator(gladiator_t *g);
gladiator_t *new_gladiator(unsigned team, double x, double y, double orientation);
void update_gladiator(gladiator_t *g, const double inputs[], double *outputs[]);
void delete_gladiator(gladiator_t *g);

#endif
