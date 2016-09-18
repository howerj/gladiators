#ifndef BRAIN_H
#define BRAIN_H

#include "util.h"

#define BRAIN_MAX_WEIGHT (2)

struct brain_t;
typedef struct brain_t brain_t;

brain_t *new_brain(prng_t *r, size_t length);
void delete_brain(brain_t *b);
void update_brain(brain_t *b, const double inputs[], size_t in_length, double outputs[], size_t out_length);

#endif
