/** @file       util.c
 *  @brief      Portable utility functions
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

#include "util.h"
#include <assert.h>
#include <ctype.h>
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
	assert(fmt);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
	abort();
}

void *allocate(size_t sz)
{
	assert(sz);
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

bool timer_tick(timer_tick_t *t)
{
	assert(t);
	if(t->i > t->max)
		return true;
	t->i++;
	return false;
}

void timer_untick(timer_tick_t *t)
{
	assert(t);
	if(t->i > 0)
		t->i--;
}

bool timer_result(timer_tick_t *t)
{
	assert(t);
	return t->i > t->max;
}

/**@todo separate out this s-expression stuff from the rest and clean it up,
 * use longjmp for error handling 
 * @todo mini-lisp interpreter 
 * @todo Move this into another library?
 * @todo schema format*/

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
	assert(l);
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
	assert(l && !(l->unget));
	l->unget = true;
	l->ungetc = c;
	if(l->ungetc == '\n')
		l->line_number--;
	return c;
}

cell_type_e type(cell_t *cell)
{
	assert(cell);
	return cell->type;
}

cell_t *car(cell_t *cons)
{
	assert(cons && type(cons) == CONS);
	cell_t *r = cons->p.cons.car;
	assert(r);
	return r;
}

cell_t *cdr(cell_t *cons)
{
	assert(cons); 
	assert(type(cons) == CONS);
	cell_t *r = cons->p.cons.cdr;
	assert(r);
	return r;
}

void setcar(cell_t *cons, cell_t *car)
{
	assert(cons && type(cons) == CONS);
	cons->p.cons.car = car;
}

void setcdr(cell_t *cons, cell_t *cdr)
{
	assert(cons && type(cons) == CONS);
	cons->p.cons.cdr = cdr;
}

cell_t *cell_new(cell_type_e type)
{
	assert(type < INVALID_CELL_TYPE);
	cell_t *c = allocate(sizeof(*c));
	c->type = type;
	c->freeable = true;
	return c;
}

cell_t *mkfloat(double x)
{
	cell_t *d = cell_new(FLOAT);
	d->p.floating = x;
	return d;
}

cell_t *mkint(int x)
{
	cell_t *d = cell_new(INTEGER);
	d->p.integer = x;
	return d;
}

cell_t *mkstr(const char *s)
{ /**@todo binary strings */
	assert(s);
	cell_t *d = cell_new(STRING);
	d->p.string = duplicate(s);
	return d;
}

cell_t *mksym(const char *s)
{ /**@todo intern strings */
	assert(s);
	cell_t *d = cell_new(SYMBOL);
	d->p.string = duplicate(s);
	return d;
}

cell_t *cons(cell_t *car, cell_t *cdr)
{
	assert(car && cdr);
	assert((type(cdr) == NIL) || (type(cdr) == CONS));
	cell_t *c = cell_new(CONS);
	c->p.cons.car = car;
	c->p.cons.cdr = cdr;
	return c;
}

cell_t *mklist(cell_t *l, ...)
{
	assert(l);
	size_t i;
	cell_t *head, *op, *next;
	va_list ap;
	head = op = cons(l, nil());
	va_start(ap, l);
	for (i = 1; (next = va_arg(ap, cell_t *)); op = cdr(op), i++)
		setcdr(op, cons(next, nil()));
	va_end(ap);
	return head;
}

cell_t *nil(void)
{ 
	static cell_t n = { .type = NIL };
	return &n;
}

bool cell_eq(cell_t *a, cell_t *b)
{
	cell_type_e at = type(a);
	if(at != type(b))
		return false;
	switch(at) {
	case NIL:
		return true;
	case INTEGER:
		return a->p.integer == b->p.integer;
	case FLOAT:
		return a->p.floating == b->p.floating;
	case SYMBOL:
	case STRING:
		return !strcmp(a->p.string, b->p.string);
	case CONS:
		return a == b;
	default:
		break;
	}
	fatal("invalid cell type: %d", (int)at);
	return false;
}

size_t cell_length(cell_t *c)
{
	assert(type(c) == CONS);
	size_t i;
	for(i = 0; c->type != NIL ; i++, c = cdr(c))
		/*do nothing*/;
	return i;
}

cell_t *nth(cell_t *c, size_t n)
{
	assert((type(c) == NIL) || type(c) == CONS);
	for(size_t i = 0; i < n && c->type != NIL; i++, c = cdr(c))
		/*do nothing*/;
	return type(c) == NIL ? c : car(c);
}

void cell_delete(cell_t *cell)
{
	if(!cell)
		return;
	switch(cell->type) {
	case NIL:
		return;
	case INTEGER:
	case FLOAT:   
		if(cell->freeable)
			free(cell); 
		return;
	case SYMBOL:  
	case STRING: 
		if(cell->freeable) {
		      free(cell->p.string);
		      free(cell);
		}
		return;
	case CONS:
		cell_delete(cell->p.cons.car);
		cell_delete(cell->p.cons.cdr);
		if(cell->freeable)
		      free(cell);
		return;
	default:
		fatal("unknown type '%u'", cell->type);
	}
}

static cell_t *parse_string(lexer_t *l)
{
	assert(l);
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
	assert(l);
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
	assert(l);
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
	assert(l);
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
	assert(input);
	lexer_t *l = lexer_new(input, NULL);
	cell_t *c = read_s_expression(l);
	lexer_delete(l);
	return c;
}

cell_t *read_s_expression_from_string(const char *input)
{
	assert(input);
	lexer_t *l = lexer_new(NULL, input);
	cell_t *c = read_s_expression(l);
	lexer_delete(l);
	return c;
}

static int print_escaped_string(const char *s, FILE *output)
{
	assert(s && output);
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

static int indent(unsigned count, FILE *output)
{
	int r = count;
	while(count--)
		if(fputc(' ', output) < 0)
			return -1;
	return r;
}

static int _write_s_expression_to_file(cell_t *cell, FILE *output, unsigned depth)
{ /**@todo basic pretty printing */
	assert(cell && output);
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
		int r = 1, f = 0;
		if(depth)
			if((f = fputc('\n', output)) < 0)
				return f;
		if((f = indent(depth, output)) < 0)
			return f;
		r += f;
		if((f = fputc('(', output)) < 0)
			return f;
		r++;
		for( ; cell->type != NIL; cell = cell->p.cons.cdr, r += f)
			if((f = _write_s_expression_to_file(cell->p.cons.car, output, depth+1)) < 0)
				return f;
		if((f = fputs(")", output)) < 0)
			return f;
		return r + f;
	}
	break;
	default:
		fatal("unknown type '%u'", cell->type);
	}
	return -1;
}

int write_s_expression_to_file(cell_t *cell, FILE *output)
{
	return _write_s_expression_to_file(cell, output, 0);
}

int scanner(cell_t *c, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = vscanner(c, fmt, ap);
	va_end(ap);
	return r;
}

static const char *type2name(cell_type_e t)
{
	assert(t >= 0 && t < INVALID_CELL_TYPE);
	static const char *name[] = {
		[NIL]     = "nil",
		[INTEGER] = "integer",
		[FLOAT]   = "float",
		[SYMBOL]  = "symbol",
		[STRING]  = "string",
		[CONS]    = "cons",
	};
	return name[t];
}

#define expect(CELL, TYPE) _expect((CELL), (TYPE), __FILE__, __func__, __LINE__)
static int _expect(cell_t *c, cell_type_e t, const char *file, const char *func, unsigned line) 
{
	if(t == type(c))
		return 1;
	fprintf(stderr, "%s:%s:%d, expected %s / got %s\n", file, func, line, type2name(t), type2name(type(c)));
	if(type(c) == STRING || type(c) == SYMBOL)
		fprintf(stderr, "%s: %s\n", type(c) == SYMBOL ? "symbol" : "string",  c->p.string);
	if(type(c) == CONS) {
		write_s_expression_to_file(c, stderr);
		fputc('\n', stderr);
	}
	return 0;
}

/** @brief Scan and validate S-Expression input
 *  @todo make a printer like version
 *
 * Example usage:
 * 	int alpha = 0;
 * 	char *bravo = NULL;
 * 	scanner(cell, "(%x (alpha %d) (bravo %s) ("charlie" %*f))", &alpha, &bravo);
 *
 *  n = nil
 *  c = cons 
 *  l = list
 *  x = any
 *  f = float
 *  d = integer
 *  s = string
 *  S = symbol
 */
int _vscanner(cell_t *c, int i, const char *fmt, va_list ap)
{
	char f;
	assert(c && fmt && ap && type(c) == CONS);
	while((f = fmt[i])) {
		if(isspace(f)) {
			i++;
			continue;
		}
		if(')' == f) {
			if(expect(c, NIL))
				return i;
			else
				return -1;
		}
		if(!expect(c, CONS))
			return -1;
		if('%' == (f = fmt[i])) {
			bool ignore = false;
			if('*' == (f = fmt[++i])) {
				i++;
				ignore = true;
			}

			cell_t *ca = car(c);
			switch((f = fmt[i])) {
			case 'l':
			case 'c':
			{
				bool list = f == 'l';
				if(!expect(list ? c : ca, CONS))
					return -1;
				if(ignore)
					break;
				cell_t **v = va_arg(ap, cell_t **);
				*v = list ? c : ca;
				if(list)
					return i;
				break;
			}
			case 'x':
			{
				cell_t **v = va_arg(ap, cell_t **);
				if(ignore)
					break;
				*v = ca;
				break;
			}
			case 'f':
			{
				if(!expect(ca, FLOAT))
					return -1;
				if(ignore)
					break;
				double *d = va_arg(ap, double *);
				*d = FLT(ca);
				break;
			}
			case 'u':
			case 'd':
			{
				if(!expect(ca, INTEGER))
					return -1;
				if(ignore)
					break;
				int *dp = va_arg(ap, int *);
				int d = INT(ca);
				*dp = d;
				if(f == 'u' && d < 0) {
					fprintf(stderr, "expected unsigned number (got %d)\n", d);
					return -1;
				}
				break;
			}
			case 's':
			{
				if(!expect(ca, STRING))
					return -1;
				if(ignore)
					break;
				char **s = va_arg(ap, char **);
				*s = STR(ca);
				break;
			}
			case 'S':
			{
				if(!expect(ca, SYMBOL))
					return -1;
				if(ignore)
					break;
				char **s = va_arg(ap, char **);
				*s = STR(c);
				break;
			}
			case 'n':
			{
				if(!expect(ca, NIL))
				break;
			}
			case '\0':
			default:
				fatal("invalid format specifier %u/%c", f, f);
			}
			i++;
		} else if('(' == f) {
			if(!expect(car(c), CONS))
				return -1;
			int r = _vscanner(car(c), ++i, fmt, ap);
			if(r < 0)
				return r;
			i += (r-i+1);
		} else {
			char s[CELL_MAX_STRING_LENGTH] = { 0 };
			/**@todo integer and float literals */
			if(f == '"') { /* string literal */
				size_t j = 0;
				if(!expect(car(c), STRING))
					return -1;
				while(strchr("\"", fmt[i])) {
					if(fmt[i] == '\\') /**@todo proper processing of this format strings */
						i++;
					s[j++] = fmt[i++];
				}
				if(strcmp(STR(car(c)), s))
					return -1;
			} else { /* symbol literal */
				size_t j = 0;
				if(!expect(car(c), SYMBOL))
					return -1;
				while(!strchr("()*%\" \t\n\r\v", fmt[i]))
					s[j++] = fmt[i++];
				if(strcmp(SYM(car(c)), s))
					return -1;
			}
		}
		c = cdr(c);
	}
	return i;
}

int vscanner(cell_t *c, const char *fmt, va_list ap)
{
	return _vscanner(c, 0, fmt, ap);
}


