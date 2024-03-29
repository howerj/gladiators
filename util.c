/** @file       util.c
 *  @brief      Portable utility functions
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/
#include "util.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

typedef struct prng_t {
	int method;
	uint64_t seed[2];
} prng_t;

static prng_t rstate;

void fatal(char *fmt, ...) {
	va_list args;
	assert(fmt);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
	abort();
}

void *allocate(size_t sz) {
	assert(sz);
	void *r = calloc(sz, 1);
	if (!r)
		fatal("allocation failed of size %zu\n", sz);
	return r;
}

char *duplicate(const char *s) {
	assert(s);
	size_t length = strlen(s) + 1;
	char *r = allocate(length);
	memcpy(r, s, length);
	return r;
}

double rad2deg(double rad) {
	return (rad / (2.0 * PI)) * 360.0;
}

double deg2rad(double deg) {
	return (deg / 360.0) * 2.0 * PI;
}

static uint64_t xorshift128(uint64_t s[2]) { /* A few rounds of SPECK or TEA ciphers also make good PRNG */
	assert(s);
	if (!s[0] && !s[1])
		s[0] = 1;
	uint64_t a = s[0];
	const uint64_t b = s[1];
	s[0] = b;
	a ^= a << 23;
	a ^= a >> 18;
	a ^= b;
	a ^= b >>  5;
	s[1] = a;
	return a + b;
}

static uint32_t temper(uint32_t x) {
    x ^= x >> 11;
    x ^= x << 7 & 0x9D2C5680;
    x ^= x << 15 & 0xEFC60000;
    x ^= x >> 18;
    return x;
}

/* from: https://stackoverflow.com/questions/19083566 */
static uint32_t lcg64_temper(uint64_t *seed) {
	assert(seed);
	*seed = 6364136223846793005ULL * *seed + 1;
	return temper(*seed >> 32);
}

void random_method(int method) {
	rstate.method = method;
}

static uint32_t prng(prng_t *state) {
	assert(state);
	if (state->method)
		return xorshift128(state->seed);
	return lcg64_temper(&state->seed[0]);
}

uint64_t random_u64(void) {
	return (((uint64_t)prng(&rstate)) << 32u) | ((uint64_t)prng(&rstate));
}

static double prngf(prng_t *state) {
	assert(state);
	double r = prng(state);
	r /= UINT32_MAX;
	return r;
}

void random_seed(double seed) {
	rstate.seed[0] = seed;
}

/* Using fixed point instead of floats throughout would have
 * had the advantage of things being far more reproducible. */
double random_float(void) {
	static bool set = false;
	if (!set) {
		rstate.seed[0] = (rstate.seed[0] != 0.0) ? rstate.seed[0] : (uint64_t)time(NULL);
		set = true;
	}
  	return prngf(&rstate);
}

/* https://stackoverflow.com/questions/11980292/how-to-wrap-around-a-range */
double wrap_rad(double rad) {
	rad = fmod(rad, 2.0 * PI);
	if (rad < 0.0)
		rad += 2.0 * PI;
	return rad;
}

bool timer_tick(timer_tick_t *t) {
	assert(t);
	if (t->i > t->max)
		return true;
	t->i++;
	return false;
}

void timer_untick(timer_tick_t *t) {
	assert(t);
	if (t->i > 0)
		t->i--;
}

bool timer_result(timer_tick_t *t) {
	assert(t);
	return t->i > t->max;
}

polar_t cartesian_to_polar(const cartesian_t c) {
	polar_t p;
	p.rho = hypot(c.x, c.y);
	p.theta = atan2(c.y, c.x);
	return p;
}

cartesian_t polar_to_cartesian(const polar_t p) {
	cartesian_t c;
	c.x = cos(p.theta) * p.rho;
	c.y = sin(p.theta) * p.rho;
	return c;
}

