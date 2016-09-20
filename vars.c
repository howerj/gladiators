#include "vars.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *default_config_file = "gladiator.conf";

#define X(TYPE, NAME, VALUE) TYPE NAME = VALUE;
CONFIG_X_MACRO
#undef X

typedef enum {
	end_e,
	double_e,
	int_e,
	unsigned_e,
	bool_e
} type_e;

static struct config_db { type_e type; void *addr; const char *name; } db[] = {
#define X(TYPE, NAME, VALUE) { TYPE ## _e , &NAME, #NAME },
CONFIG_X_MACRO
#undef X
	{ end_e, NULL, NULL }
};

bool verbose(verbosity_t v)
{
	return verbosity >= v;
}

size_t find_config_item(const char* item)
{
	size_t i;
	for(i = 0; db[i].type != end_e; i++) {
		if(!strcmp(db[i].name, item))
			break;
	}
	return i;
}

bool load_config(void)
{
	FILE *in = fopen(default_config_file, "rb");
	if(!in) {
		fprintf(stderr, "configuration file '%s' failed to load\n", default_config_file);
		return false;
	}
	char item[512] = { 0 };

	while(fscanf(in, "%511s", item) > 0) {
		size_t i = find_config_item(item);
		if(db[i].type == end_e) {
			fprintf(stderr, "unknown configuration item '%s'\n", item);
			goto end;
		}
		switch(db[i].type) {
		case double_e:   fscanf(in, "%lf\n", (double*)db[i].addr);  break;
		case bool_e:     fscanf(in, "%u\n", (unsigned*)db[i].addr); break;
		case int_e:      fscanf(in, "%d\n", (int*)db[i].addr);      break;
		case unsigned_e: fscanf(in, "%u\n", (unsigned*)db[i].addr); break;
		case end_e:      break;
		default:
			fprintf(stderr, "invalid configuration item type '%d'\n", db[i].type);
			exit(EXIT_FAILURE);

		}

		memset(item, 0, sizeof(item));
	}

end:
	fclose(in);
	return false;
}

bool save_config(void)
{
	FILE *out = fopen(default_config_file, "wb");
	if(!out) {
		fprintf(stderr, "configuration file '%s' failed to save\n", default_config_file);
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
			fprintf(stderr, "invalid configuration item type '%d'\n", db[i].type);
			exit(EXIT_FAILURE);
		}
	}
	fclose(out);
	return true;
}

