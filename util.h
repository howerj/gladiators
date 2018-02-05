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
#include <stdint.h>

#define UNUSED(X) ((void)(X))

#define PI  (3.1415926535897932384626433832795)
#define ESC (27) /*escape key*/

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

typedef struct {
	unsigned i;
	unsigned max;
} timer_tick_t;

typedef struct {
	double rho;
	double theta;
} polar_t;

typedef struct {
	double x;
	double y;
} cartesian_t;

void fatal(char *fmt, ...);
void *allocate(size_t sz);
char *duplicate(const char *s);
double rad2deg(double rad);
double deg2rad(double deg);
void random_seed(double seed);
double random_float(void);
uint64_t random_u64(void);


double wrap_or_limit_x(double x);
double wrap_or_limit_y(double y);
double wrap_rad(double rad);

bool timer_tick(timer_tick_t *t);
void timer_untick(timer_tick_t *t);
bool timer_result(timer_tick_t *t);

polar_t cartesian_to_polar(const cartesian_t c);
cartesian_t polar_to_cartesian(polar_t t);

unsigned binary_logarithm_base_2(unsigned x);

#endif
