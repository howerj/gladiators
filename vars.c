#include "vars.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

const char *default_config_file = "gladiator.conf";

#define X(TYPE, NAME, VALUE, ZERO_ALLOWED) TYPE NAME = VALUE;
CONFIG_X_MACRO
#undef X

typedef enum {
	end_e,
	double_e,
	int_e,
	unsigned_e,
	bool_e
} type_e;

static struct config_db { type_e type; void *addr; const char *name; const bool zero_allowed; } db[] = {
#define X(TYPE, NAME, VALUE, ZERO_ALLOWED) { TYPE ## _e , &NAME, #NAME, ZERO_ALLOWED },
CONFIG_X_MACRO
#undef X
	{ end_e, NULL, NULL, false }
};

void error(const char *fmt, ...)
{
	va_list args;
	if(!verbose(ERROR))
		return;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
	exit(EXIT_FAILURE);
}

void warning(const char *fmt, ...)
{
	va_list args;
	if(!verbose(WARNING))
		return;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
}

bool verbose(verbosity_t v)
{
	return program_log_level >= v;
}

static bool is_bool_valid(struct config_db *db)
{
	unsigned b = *(unsigned*)(db->addr);
	return !(b > 1 || (!(db->zero_allowed) && !b));
}

static bool is_double_valid(struct config_db *db)
{
	double d = *(double*)(db->addr); 
	return !(isnan(d) || isinf(d) || (!(db->zero_allowed) && !d));
}

static bool is_int_valid(struct config_db *db)
{
	int i = *(int*)(db->addr);
	return !(!(db->zero_allowed) && !i);
}

static bool is_unsigned_valid(struct config_db *db)
{
	unsigned u = *(unsigned*)(db->addr);
	return !(!(db->zero_allowed) && !u);
}

static bool validate_config(void)
{
	bool config_is_valid = true;
	for(int i = 0; db[i].type != end_e; i++) {
		bool valid = true;
		switch(db[i].type) {
		case double_e:   valid = is_double_valid(&db[i]);   break;
		case bool_e:     valid = is_bool_valid(&db[i]);     break;
		case int_e:      valid = is_int_valid(&db[i]);      break;
		case unsigned_e: valid = is_unsigned_valid(&db[i]); break;
		case end_e:                                         break;
		default:         error("invalid configuration item type '%d'", db[i].type);
		}
		if(!valid) {
			warning("invalid configuration value item '%s'", db[i].name);
			config_is_valid = false;
		}
	}
	return config_is_valid;
}

size_t find_config_item(const char* item)
{
	size_t i;
	for(i = 0; db[i].type != end_e; i++)
		if(!strcmp(db[i].name, item))
			break;
	return i;
}

void load_config(void)
{
	FILE *in = fopen(default_config_file, "rb");
	char item[512] = { 0 };
	if(!in) {
		warning("configuration file '%s' failed to load: using default values", default_config_file);
		return;
	}

	while(fscanf(in, "%511s", item) > 0) {
		size_t i = find_config_item(item);
		if(db[i].type == end_e)
			error("unknown configuration item '%s'", item);
		switch(db[i].type) {
		case double_e:   fscanf(in, "%lf\n", (double*)db[i].addr);  break;
		case bool_e:     fscanf(in, "%u\n",  (unsigned*)db[i].addr); break;
		case int_e:      fscanf(in, "%d\n",  (int*)db[i].addr);      break;
		case unsigned_e: fscanf(in, "%u\n",  (unsigned*)db[i].addr); break;
		case end_e:      break;
		default:         error("invalid configuration item type '%d'", db[i].type);
		}
		memset(item, 0, sizeof(item));
	}
	if(!validate_config())
		error("invalid configuration file: to regenerate a valid configuration use '-s'");
	fclose(in);
}

bool save_config(void)
{
	FILE *out = fopen(default_config_file, "wb");
	if(!out) {
		warning("failed to save configuration file '%s'", default_config_file);
		return false;
	}
	for(size_t i = 0; db[i].type != end_e; i++) {
		switch(db[i].type) {
		case double_e:   fprintf(out, "%s %f\n", db[i].name, *(double*)db[i].addr);   break;
		case bool_e:     fprintf(out, "%s %u\n", db[i].name, *(bool*)db[i].addr);     break;
		case int_e:      fprintf(out, "%s %d\n", db[i].name, *(int*)db[i].addr);      break;
		case unsigned_e: fprintf(out, "%s %u\n", db[i].name, *(unsigned*)db[i].addr); break;
		case end_e:      break;
		default:
			error("invalid configuration item type '%d'", db[i].type);
		}
	}
	fclose(out);
	return true;
}

