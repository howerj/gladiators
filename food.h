/** @file       food.h
 *  @brief      Food object, gladiators can eat this
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

#ifndef FOOD_H
#define FOOD_H

#include <stdbool.h>
#include "sexpr.h"

typedef enum {
	FOOD_RANDOM_WALK_E,
	FOOD_BOUNCE_E
} food_control_method_t;

typedef struct {
	double x, y;
	double radius;
	double orientation;
	bool eaten;
} food_t;

food_t *food_new(double x, double y, double orientation);
void food_delete(food_t *f);
void food_draw(food_t *f);
void food_update(food_t *f);
bool food_is_active(food_t *f);
void food_reactivate(food_t *f, double x, double y, double orientation);
void food_deactive(food_t *f);
cell_t *food_serialize(food_t *f);
food_t *food_deserialize(cell_t *c);

#endif
