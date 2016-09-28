/** @file       util.h
 *  @brief      Portable utility functions
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

#ifndef UTIL_H
#define UTIL_H

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#define UNUSED(X) ((void)(X))

#define PI  (3.1415926535897932384626433832795)
#define ESC (27) /*escape key*/

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

typedef struct {
	unsigned i;
	unsigned max;
} tick_timer_t;

void *allocate(size_t sz);
char *duplicate(const char *s);
double rad2deg(double rad);
double deg2rad(double deg);
void random_seed(double seed);
double random_float(void);

double wrap_or_limit_x(double x);
double wrap_or_limit_y(double y);
double wrap_rad(double rad);

bool tick_timer(tick_timer_t *t);
bool tick_result(tick_timer_t *t);

typedef enum {
	NIL,
	INTEGER,
	FLOAT,
	SYMBOL,
	STRING,
	CONS,
	INVALID_CELL_TYPE
} cell_type_e;

#define CELL_MAX_STRING_LENGTH (256)

typedef struct cell_t {
	cell_type_e type;
	union data{
		struct cons_t {
			struct cell_t *car;
			struct cell_t *cdr;
		} cons;
		char *string;
		int integer;
		double floating;
	} p;
} cell_t;

cell_type_e type(cell_t *cell);
cell_t *car(cell_t *cons);
cell_t *cdr(cell_t *cons);
void setcar(cell_t *cons, cell_t *car);
void setcdr(cell_t *cons, cell_t *cdr);
cell_t *cons(cell_t *car, cell_t *cdr);
void cell_delete(cell_t *cell);
cell_t *read_s_expression_from_file(FILE *input);
cell_t *read_s_expression_from_string(const char *input);
int write_s_expression_to_file(cell_t *cell, FILE *output);
cell_t *cons(cell_t *car, cell_t *cdr);
cell_t *nil(void);
cell_t *mkfloat(double x);
cell_t *mkint(int x);
cell_t *mkstr(const char *s);
cell_t *mksym(const char *s);
cell_t *mklist(cell_t *l, ...);

#endif
