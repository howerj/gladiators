#ifndef BRAIN_H
#define BRAIN_H

#include <stdio.h>
#include <stdbool.h>

struct brain_t;
typedef struct brain_t brain_t;

brain_t *brain_new(bool rand, size_t length);
brain_t *brain_copy(const brain_t *b);
void brain_delete(brain_t *b);
void brain_update(brain_t *b, const double inputs[], size_t in_length, double outputs[], size_t out_length);
void brain_mutate(brain_t *b);
void brain_print(FILE *output, brain_t *b);

#endif
