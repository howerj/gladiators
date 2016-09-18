#ifndef PROJECTILE_H
#define PROJECTILE_H

#define PROJECTILE_DISTANCE_PER_TICK (1)
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

projectile_t *new_projectile(unsigned team, double x, double y, double orientation);
void draw_projectile(projectile_t *p);
void update_projectile(projectile_t *p);
bool is_projectile_active(projectile_t *p);
bool fire_projectile(projectile_t *p, double x, double y, double orientation);

#endif
