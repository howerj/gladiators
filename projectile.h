#ifndef PROJECTILE_H
#define PROJECTILE_H

#define PROJECTILE_DISTANCE_PER_TICK (2)
#define PROJECTILE_RANGE             (60)
#define PROJECTILE_SIZE              (1)
#define PROJECTILE_DAMAGE            (1)

#include <stdbool.h>

typedef struct {
	double x, y;
	double radius;
	double orientation;
	double travelled;
	unsigned team;
} projectile_t;

projectile_t *projectile_new(unsigned team, double x, double y, double orientation);
void projectile_delete(projectile_t *p);
void projectile_draw(projectile_t *p);
void projectile_update(projectile_t *p);
bool projectile_is_active(projectile_t *p);
bool projectile_fire(projectile_t *p, double x, double y, double orientation);

#endif
