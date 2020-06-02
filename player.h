#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <stdint.h>
#include "sexpr.h"

typedef struct {
	double x, y;
	double orientation;
	double health;
	unsigned team;
	unsigned hits;
	unsigned foods;
	unsigned refire_timeout;
	double energy;
	double score;
	double radius;
} player_t;

void player_draw(player_t *p);
player_t *player_new(unsigned team);
void player_delete(player_t *p);
void player_update(player_t *p, bool fire, bool left, bool right, bool forward);
bool player_is_dead(player_t *p);
cell_t *player_serialize(player_t *p);
player_t *player_deserialize(cell_t *c);

#endif
