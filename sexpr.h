/** @file       sexpr.h
 *  @brief      S-Expression parsing and manipulation
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

#ifndef SEXPR_H
#define SEXPR_H

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
	NIL,
	INTEGER,
	FLOATING,
	SYMBOL,
	STRING,
	CONS,
	INVALID_CELL_TYPE
} cell_type_e;

#define CELL_MAX_STRING_LENGTH (256u)

typedef struct cell_t {
	union data {
		struct cons_t {
			struct cell_t *car;
			struct cell_t *cdr;
		} cons;
		char *string;
		intptr_t integer;
		double floating;
	} p;
	cell_type_e type    : 8;
	unsigned    freeable: 1;
} cell_t;

#define STR(CELL) (assert(CELL->type == STRING),   (CELL)->p.string)
#define SYM(CELL) (assert(CELL->type == SYMBOL),   (CELL)->p.string)
#define INT(CELL) (assert(CELL->type == INTEGER),  (CELL)->p.integer)
#define FLT(CELL) (assert(CELL->type == FLOATING), (CELL)->p.floating)

#define CAAR(CELL) (car(car(CELL)))
#define CADR(CELL) (car(cdr(CELL)))
#define CDAR(CELL) (cdr(car(CELL)))
#define CDDR(CELL) (cdr(cdr(CELL)))

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
size_t cell_length(cell_t *c);
bool cell_eq(cell_t *a, cell_t *b);
cell_t *nth(cell_t *c, size_t n);
cell_t *mkfloat(double x);
cell_t *mkint(intptr_t x);
cell_t *mkstr(const char *s);
cell_t *mksym(const char *s);
cell_t *mklist(cell_t *l, ...);
int scanner(cell_t *c, const char *fmt, ...);
int vscanner(cell_t *c, const char *fmt, va_list ap);
cell_t *printer(const char *fmt, ...);
cell_t *vprinter(const char *fmt, va_list ap);

#endif
