/** @file       vars.c
 *  @brief      Global configuration variables and management
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com
 *  @todo       Convert to use S-expressions for the format*/

#include "vars.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

const char *default_config_file = "gladiator.conf";

#define X(TYPE, NAME, VALUE, MINIMUM, MAXIMUM, DESCIPTION) TYPE NAME = VALUE;
CONFIG_X_MACRO
#undef X

typedef enum {
	end_e,
	double_e,
	int_e,
	unsigned_e,
	bool_e,
} type_e;

typedef struct {
	type_e type; 
	void *addr; 
	const char *name; 
	double min, max;
	const char *description;
} config_db_t;

static config_db_t db[] = {
#define X(TYPE, NAME, VALUE, MINIMUM, MAXIMUM, DESCIPTION) { TYPE ## _e , &NAME, #NAME, MINIMUM, MAXIMUM, DESCIPTION },
CONFIG_X_MACRO
#undef X
	{ end_e, NULL, NULL, 0, 0, NULL }
};

static const bool logging_prepend_level = true;

#define MESSAGE(PREPEND, FMT, LEVEL)\
	do {\
		assert(FMT);\
		if (!verbose(LEVEL))\
			return;\
		if (logging_prepend_level)\
			fputs((PREPEND), stderr);\
		va_list args;\
		va_start(args, FMT);\
		vfprintf(stderr, (FMT), args);\
		va_end(args);\
		fputc('\n', stderr);\
	} while (0);

void error(const char *fmt, ...) {
	assert(fmt);
	if (!verbose(ERROR))
		exit(EXIT_FAILURE);
	MESSAGE("error: ", fmt, ERROR);
	exit(EXIT_FAILURE);
}

void warning(const char *fmt, ...) {
	MESSAGE("warning: ", fmt, WARNING);
}

void note(const char *fmt, ...) {
	MESSAGE("note: ", fmt, NOTE);
}

void debug(const char *fmt, ...) {
	MESSAGE("debug: ", fmt, DEBUG);
}

bool verbose(verbosity_t v) {
	return program_log_level >= v;
}

static bool within(double val, double min, double max) {
	return val >= min && val <= max;
}

static bool is_bool_valid(config_db_t *db) {
	bool b = *(bool*)(db->addr);
	return (b == 1 || b == 0) && within(b, db->min, db->max);
}

static bool is_double_valid(config_db_t *db) {
	double d = *(double*)(db->addr);
	return !(isnan(d) || isinf(d)) && within(d, db->min, db->max);
}

static bool is_int_valid(config_db_t *db) {
	int i = *(int*)(db->addr);
	return within(i, db->min, db->max);
}

static bool is_unsigned_valid(config_db_t *db) {
	unsigned u = *(unsigned*)(db->addr);
	return within(u, db->min, db->max);
}

static bool config_validate(void) {
	bool config_is_valid = true;
	for (int i = 0; db[i].type != end_e; i++) {
		assert(db[i].addr);
		bool valid = true;
		switch (db[i].type) {
		case double_e:   valid = is_double_valid(&db[i]);   break;
		case bool_e:     valid = is_bool_valid(&db[i]);     break;
		case int_e:      valid = is_int_valid(&db[i]);      break;
		case unsigned_e: valid = is_unsigned_valid(&db[i]); break;
		case end_e:                                         break;
		default:         error("invalid configuration item type '%d'", db[i].type);
		}
		if (!valid) {
			warning("invalid configuration value item '%s'", db[i].name);
			config_is_valid = false;
		}
	}
	return config_is_valid;
}

static size_t find_config_item(const char* item) {
	size_t i;
	for (i = 0; db[i].type != end_e; i++)
		if (!strcmp(db[i].name, item))
			break;
	return i;
}

int config_load(void) {
	char item[512] = { 0 };
	FILE *in = fopen(default_config_file, "rb");
	if (!in) {
		static const char *msg = "configuration file '%s' failed to load: using default values\nuse '-s' to generate it.";
		debug(msg, default_config_file);
		return -1;
	}

	while (fscanf(in, "%511s", item) > 0) {
		size_t i = find_config_item(item);
		if (db[i].type == end_e)
			error("unknown configuration item '%s'", item);
		assert(db[i].addr);
		unsigned b = 0;
		int r = 0;
		switch (db[i].type) {
		case double_e:   r = fscanf(in, "%lf\n", (double*)db[i].addr);          break;
		case bool_e:     r = fscanf(in, "%u\n",  &b); *((bool*)db[i].addr) = b; break;
		case int_e:      r = fscanf(in, "%d\n",  (int*)db[i].addr);             break;
		case unsigned_e: r = fscanf(in, "%u\n",  (unsigned*)db[i].addr);        break;
		case end_e:      break;
		default:         error("invalid configuration item type '%d'", db[i].type);
		}
		if (r != 1)
			error("could not scan input token of type '%u'", (unsigned)(db[i].type));
		memset(item, 0, sizeof(item));
	}
	if (!config_validate())
		error("invalid configuration file: to regenerate a valid configuration use '-s'");
	if (fclose(in) < 0)
		return -1;
	return 0;
}

int config_save_to_default_config_file(void) {
	FILE *out = fopen(default_config_file, "wb");
	if (!out) {
		debug("failed to save configuration file '%s'", default_config_file);
		return -1;
	}
	const int r = config_save(out);
	if (fclose(out) < 0)
		return -1;
	return r;
}

int config_save(FILE *out) {
	assert(out);
	for (size_t i = 0; db[i].type != end_e; i++) {
		int r = 0;
		assert(db[i].addr);
		switch (db[i].type) {
		case double_e:   r = fprintf(out, "%s %f\n", db[i].name, *(double*)db[i].addr);   break;
		case bool_e:     r = fprintf(out, "%s %u\n", db[i].name, *(bool*)db[i].addr);     break;
		case int_e:      r = fprintf(out, "%s %d\n", db[i].name, *(int*)db[i].addr);      break;
		case unsigned_e: r = fprintf(out, "%s %u\n", db[i].name, *(unsigned*)db[i].addr); break;
		case end_e:      break;
		default: error("invalid configuration item type '%d'", db[i].type);
		}
		if (r < 0) {
			warning("configuration saving failed: %d\n", r);
			return -1;
		}
	}
	return 0;
}

static const char *type_lookup(type_e type) {
	const char *r = "unknown";
	switch (type) {
	case end_e:       r = "end";      break;
	case double_e:    r = "double";   break;
	case int_e:       r = "int";      break;
	case unsigned_e:  r = "unsigned"; break;
	case bool_e:      r = "boolean";  break;
	}
	return r;
}

int config_help(FILE *out) {
	assert(out);
	if (fputs("\nConfiguration Items\n", out) < 0)
		return -1;
	for (size_t i = 0; db[i].type != end_e; i++) {
		const int r = fprintf(out, "* \"%s\" : type = %s, minimum value = %g, maximum value = %g\n\n%s\n\n", 
				db[i].name, type_lookup(db[i].type), db[i].min, db[i].max, db[i].description);
		if (r < 0)
			return -1;
	}
	return 0;
}

cell_t *config_serialize(void) {
	cell_t *cfg = cons(mksym("configuration"), nil());
	for (size_t i = 0; db[i].type != end_e; i++) {
		intptr_t p = 0;
		double d = 0;
		bool floating = 0;
		switch (db[i].type) {
		case double_e:   d = *(double*)db[i].addr; floating = 1; break;
		case bool_e:     p = *(bool*)db[i].addr;                 break;
		case int_e:      p = *(int*)db[i].addr;                  break;
		case unsigned_e: p = *(unsigned*)db[i].addr;             break;
		case end_e:      break;
		default: error("invalid configuration item type '%d'", db[i].type);
		}
		cell_t *v = floating ? mkfloat(d) : mkint(p);
		cell_t *dbi = printer("item %s %x ", db[i].name, v);
		cell_t *cns = cons(dbi, cdr(cfg));
		setcdr(cfg, cns);
	}
	return cfg;
}

int config_deserialize(cell_t *c) {
	assert(c);
	cell_t *items = NULL;
	if (scanner(c, "configuration %x", &items) < 0) {
		warning("no configuration");
		goto fail;
	}
	items = cdr(c);
	for (; type(items) != NIL; items = cdr(items)) {
		cell_t *v = NULL;
		char *name = NULL;
		if (scanner(car(items), "item %s %x", &name, &v) < 0) {
			warning("invalid item");
			goto fail;
		}
		int vtype = type(v);
		if (vtype != INTEGER && vtype != FLOATING) {
			warning("invalid item type for '%s'", name);
			goto fail;
		}
		size_t i = find_config_item(name);
		if (db[i].type == end_e) {
			warning("unknown configuration item '%s'", name);
			goto fail;
		}
		int floating = vtype == FLOATING;
		double d = 0;
		intptr_t p = 0;
		if (floating)
			d = v->p.floating;
		else
			p = v->p.integer;
		if (floating && db[i].type != double_e) {
			warning("type mismatch in config item '%s'", name);
			goto fail;
		}

		switch (db[i].type) {
		case double_e:   *(double*)db[i].addr = d;   break;
		case bool_e:     *(bool*)db[i].addr = !!p;   break;
		case int_e:      *(int*)db[i].addr = p;      break;
		case unsigned_e: *(unsigned*)db[i].addr = p; break;
		case end_e:      break;
		default: error("invalid configuration item type '%d'", db[i].type);
		}
	}
	return 0;
fail:
	return -1;
}

