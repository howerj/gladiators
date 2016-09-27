/** @file       util.c
 *  @brief      Portable utility functions
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

#include "util.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>

struct prng_t {
	uint64_t seed;
};
typedef struct prng_t prng_t;

static void fatal(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
	abort();
}

void *allocate(size_t sz)
{
	void *r = calloc(sz, 1);
	if(!r)
		fatal("allocation failed of size %zu\n", sz);
	return r;
}

char *duplicate(const char *s)
{
	size_t length = strlen(s);
	char *r = allocate(length);
	memcpy(r, s, length);
	return r;
}

double rad2deg(double rad)
{
	return (rad / (2.0 * PI)) * 360.0;
}

double deg2rad(double deg)
{
	return (deg / 360.0) * 2.0 * PI;
}

static uint32_t temper(uint32_t x)
{
    x ^= x>>11;
    x ^= x<<7 & 0x9D2C5680;
    x ^= x<<15 & 0xEFC60000;
    x ^= x>>18;
    return x;
}

/* from: https://stackoverflow.com/questions/19083566 */
static uint32_t lcg64_temper(uint64_t *seed)
{
	assert(seed);
	*seed = 6364136223846793005ULL * *seed + 1;
	return temper(*seed >> 32);
}

static uint32_t prng(prng_t *state)
{
	assert(state);
	return lcg64_temper(&state->seed);
}

static double prngf(prng_t *state) 
{
	assert(state);
	double r = prng(state);
	r /= UINT32_MAX;
	return r;
}

static prng_t rstate;
void random_seed(double seed)
{
	rstate.seed = seed;
}

double random_float(void)
{
	static bool set = false;
	if(!set) {
		rstate.seed = (rstate.seed != 0.0) ? rstate.seed : (uint64_t)time(NULL);
		set = true;
	}
  	return prngf(&rstate);
}

double wrap_rad(double rad)
{ /* https://stackoverflow.com/questions/11980292/how-to-wrap-around-a-range */
	rad = fmod(rad, 2.0 * PI);
	if (rad < 0.0)
		rad += 2.0 * PI;
	return rad;
}

bool tick_timer(tick_timer_t *t)
{
	if(t->i > t->max)
		return true;
	t->i++;
	return false;
}

bool tick_result(tick_timer_t *t)
{
	return t->i > t->max;
}

/**@todo separate out this s-expression stuff from the rest and clean it up */

typedef struct {
	unsigned line_number;
	bool string_input;
	const char *s;
	bool unget;
	int ungetc;
	FILE *f;
} lexer_t;

static lexer_t *lexer_new(FILE *fin, const char *sin)
{
	assert(!!fin ^ !!sin);
	lexer_t *l = allocate(sizeof(*l));
	l->s = sin;
	l->f = fin;
	l->string_input = !!sin;
	return l;
}

static void lexer_delete(lexer_t *l)
{
	free(l);
}

static int get_char(lexer_t *l)
{
	int c;
	if(l->unget) {
		l->unget = false;
		c = l->ungetc;
	}
	else if(l->string_input) {
		if(!(c = *(l->s++)))
			return EOF;
	} else {
		c = fgetc(l->f);
	}
	if(c == '\n')
		l->line_number++;
	return c;
}

static int unget_char(lexer_t *l, int c)
{
	if(l->unget)
		fprintf(stderr, "unget twice in a row (%u/%d/%d)", l->line_number, l->ungetc, c);
	l->unget = true;
	l->ungetc = c;
	if(l->ungetc == '\n')
		l->line_number--;
	return c;
}

cell_t *cell_new(cell_type_e type)
{
	assert(type < INVALID_CELL_TYPE);
	cell_t *c = allocate(sizeof(*c));
	c->type = type;
	return c;
}

cell_t *cons(cell_t *car, cell_t *cdr)
{
	assert(car && cdr);
	cell_t *c = cell_new(CONS);
	c->p.cons.car = car;
	c->p.cons.cdr = cdr;
	return c;
}

void cell_delete(cell_t *cell)
{
	if(!cell)
		return;
	switch(cell->type) {
	case NIL:
	case INTEGER:
	case FLOAT:   free(cell); 
		      return;
	case SYMBOL:  
	case STRING: 
		      free(cell->p.string);
		      free(cell);
		      return;
	case CONS:
		      cell_delete(cell->p.cons.car);
		      cell_delete(cell->p.cons.cdr);
		      free(cell);
		      return;
	default:
		fatal("unknown type '%u'", cell->type);
	}
}

static cell_t *parse_string(lexer_t *l)
{
	char s[CELL_MAX_STRING_LENGTH] = {0};
	cell_t *c = cell_new(STRING);
	for(size_t i = 0; i < CELL_MAX_STRING_LENGTH - 1; i++) {
		int ch = get_char(l);
		switch(ch) {
		case EOF:
			fprintf(stderr, "unexpected EOF on line %u\n", l->line_number);
			goto fail;
		case '"':
			c->p.string = duplicate(s);
			return c;
		case '\\':
		{
			int escape = get_char(l);
			switch(escape) {
			case '\\':
			case '"':
				s[i] = escape;
				break;
			case 'n':
				s[i] = '\n';
				break;
			default:
				fprintf(stderr, "invalid escaped char '%u'", ch);
				goto fail;
			}
			break;
		}
		default:
			s[i] = ch;
		}
	}
	fprintf(stderr, "max string length %u exceeded on line %u\n", CELL_MAX_STRING_LENGTH, l->line_number);
fail:
	cell_delete(c);
	return NULL;
}

static cell_t *parse_symbol_or_number(lexer_t *l)
{
	char s[CELL_MAX_STRING_LENGTH] = {0};
	cell_t *c = cell_new(SYMBOL);
	for(size_t i = 0; i < CELL_MAX_STRING_LENGTH - 1; i++) {
		int ch = get_char(l);
		switch(ch) {
		case EOF:
			fprintf(stderr, "unexpected EOF on line %u\n", l->line_number);
			goto fail;
		case ' ':
		case '\n':
		case '\t':
		case '(':
		case ')':
		{
			unget_char(l, ch);
			char *end = s;
			errno = 0;
			c->p.integer = strtol(s, &end, 0);
			if(!*end && !errno) {
				c->type = INTEGER;
				return c;
			}
			end = s;
			errno = 0;
			c->p.floating = strtod(s, &end);
			if(!*end && !errno) {
				c->type = FLOAT;
				return c;
			}
			c->p.string = duplicate(s);
			return c;
		}
		default:
			s[i] = ch;
		}
	}
	fprintf(stderr, "max string length %u exceeded on line %u\n", CELL_MAX_STRING_LENGTH, l->line_number);
fail:
	cell_delete(c);
	return NULL;

}

static cell_t *read_s_expression(lexer_t *l);

static cell_t *parse_list(lexer_t *l)
{
	int ch;
again:
	ch = get_char(l);
	if(strchr(" \n\t", ch))
		goto again;
	if(ch == ')') {
		return cell_new(NIL);
	} else if(ch == EOF) {
		fprintf(stderr, "unexpected EOF in list\n");
		return NULL;
	} else {
		cell_t *c = cell_new(CONS);
		unget_char(l, ch);
		if(!(c->p.cons.car = read_s_expression(l))) {
			cell_delete(c);
			fprintf(stderr, "unexpected NULL in list\n");
			return NULL;
		}
		c->p.cons.cdr = parse_list(l);
		return c;
	}
}

static cell_t *read_s_expression(lexer_t *l)
{
	int ch;
again:
	ch = get_char(l);
	if(strchr(" \n\t", ch))
		goto again;
	switch(ch) {
	case EOF:  return NULL;
	case '(':
		   return parse_list(l);
	case ')':
		   fprintf(stderr, "unexpected ')' on line %u\n", l->line_number);
		   return NULL;
	case '"':
		   return parse_string(l);
	default:
		   unget_char(l, ch);
		   return parse_symbol_or_number(l);
	}
	return NULL;
}

cell_t *read_s_expression_from_file(FILE *input)
{
	lexer_t *l = lexer_new(input, NULL);
	cell_t *c = read_s_expression(l);
	lexer_delete(l);
	return c;
}

cell_t *read_s_expression_from_string(const char *input)
{
	lexer_t *l = lexer_new(NULL, input);
	cell_t *c = read_s_expression(l);
	lexer_delete(l);
	return c;
}

static int print_escaped_string(const char *s, FILE *output)
{
	int r = 1, f = 0;
	if((f = fputc('"', output)) < 0)
		return f;
	for(size_t i = 0; s[i]; i++)
		switch(s[i]) {
		case '\\':
			if((f = fputs("\\\\", output)) < 0)
				return f;
			r += f;
			break;
		case '\"':
			if((f = fputs("\\\"", output)) < 0)
				return f;
			r += f;
			break;
		case '\n':
			if((f = fputs("\\n", output)) < 0)
				return f;
			r += f;
			break;
		default:
			if((f = fputc(s[i], output)) < 0)
				return f;
			r += 1;
			break;
		}
	if((f = fputs("\" ", output)) < 0)
		return f;
	r += f;
	return r;
}

int write_s_expression_to_file(cell_t *cell, FILE *output)
{
	if(!cell)
		fatal("unexpected NULL");
	switch(cell->type) {
	case NIL:     return fprintf(output, "() ");
	case INTEGER: return fprintf(output, "%d ", cell->p.integer);
	case FLOAT:   return fprintf(output, "%.3lf ", cell->p.floating);
	case SYMBOL:  return fprintf(output, "%s ", cell->p.string);
	case STRING:  return print_escaped_string(cell->p.string, output);
	case CONS:
	{
		int r = 0, f = 0;
		if((f = fputc('(', output)) < 0)
			return f;
		r++;
		for( ; cell->type != NIL; cell = cell->p.cons.cdr, r += f)
			if((f = write_s_expression_to_file(cell->p.cons.car, output)) < 0)
				return f;
		if((f = fputs(") ", output)) < 0)
			return f;
		return r + f;
	}
	break;
	default:
		fatal("unknown type '%u'", cell->type);
	}
	return -1;
}

