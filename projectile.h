/** @file       projectile.h
 *  @brief      projectile object, gladiators can fire these
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <stdbool.h>
#include <stdint.h>
#include "sexpr.h"

typedef struct {
	double x, y;
	double orientation;
	double radius;
	double travelled;
	intptr_t team;
} projectile_t;

projectile_t *projectile_new(unsigned team, double x, double y, double orientation);
void projectile_delete(projectile_t *p);
void projectile_draw(projectile_t *p);
void projectile_update(projectile_t *p);
bool projectile_is_active(projectile_t *p);
bool projectile_fire(projectile_t *p, double x, double y, double orientation);
void projectile_deactivate(projectile_t *p);
cell_t *projectile_serialize(projectile_t *p);
projectile_t *projectile_deserialize(cell_t *c);

#endif
