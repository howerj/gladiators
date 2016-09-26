/** @file       brain.h
 *  @brief      
 *  @author     Richard Howe (2016)
 *  @license    LGPL v2.1 or Later 
 *              <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html> 
 *  @email      howe.r.j.89@gmail.com*/

#ifndef BRAIN_H
#define BRAIN_H

#include <stdio.h>
#include <stdbool.h>

struct brain_t;
typedef struct brain_t brain_t;

typedef enum {
	/* see https://en.wikipedia.org/wiki/Activation_function for more
	 * functions */
	LOGISTIC_FUNCTION_E,
	TANH_FUNCTION_E,
	ATAN_FUNCTION_E,
	IDENTITY_FUNCTION_E,
	BINARY_STEP_FUNCTION_E,
	RECTIFIER_FUNCTION_E,
	SIN_FUNCTION_E,
} activation_function_t;

brain_t *brain_new(bool rand, size_t length, size_t depth);
brain_t *brain_copy(const brain_t *b);
void brain_delete(brain_t *b);
void brain_update(brain_t *b, const double inputs[], size_t in_length, double outputs[], size_t out_length);
unsigned brain_mutate(brain_t *b);
int brain_save(FILE *output, brain_t *b);
brain_t *brain_load(FILE *input);
brain_t *brain_crossover(brain_t *a, brain_t *b);

#endif
