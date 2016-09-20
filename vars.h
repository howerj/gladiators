#ifndef VARS_H
#define VARS_H

#include <stdbool.h>

typedef enum {
	ALL_MESSAGES_OFF,
	NOTE,
	WARNING,
	DEBUG,
	ALL_MESSAGES_ON,
} verbosity_t;

#define CONFIG_X_MACRO\
	X(unsigned,  verbosity,                     NOTE)\
	X(bool,      wrap_mode,                     false)\
	X(double,    Xmin,                          0.0)\
	X(double,    Xmax,                          200.0)\
	X(double,    Ymin,                          0.0)\
	X(double,    Ymax,                          200.0)\
	X(double,    window_width,                  500.0)\
	X(double,    window_height,                 500.0)\
	X(double,    window_x,                      60.0)\
	X(double,    window_y,                      20.0)\
	X(double,    tick_ms,                       15.0)\
	X(unsigned,  max_gladiators,                5)\
	X(double,    max_ticks_per_generation,      4000.0)\
	X(double,    detection_lines,               100.0)\
	X(double,    mutation_rate,                 0.15)\
	X(double,    gladiator_size,                3.0)\
	X(double,    gladiator_health,              2.0)\
	X(double,    gladiator_fire_timeout,        15.0)\
	X(unsigned,  gladiator_brain_length,        10)\
	X(unsigned,  gladiator_brain_depth,         2)\
	X(double,    gladiator_distance_per_tick,   1.0)\
	X(double,    gladiator_fire_threshold,      0.50)\
	X(double,    gladiator_vision,              120.0)\
	X(double,    gladiator_turn_rate_divisor,   12.0)\
	X(double,    projectile_distance_per_tick,  2.0)\
	X(double,    projectile_range,              250.0)\
	X(double,    projectile_size,               1.0)\
	X(double,    projectile_damage,             1.0)\
	X(double,    projectile_energy_cost,        150.0)\
	X(double,    fitness_weight_health,         1.5)\
	X(double,    fitness_weight_hits,           1.0)\
	X(double,    fitness_weight_rank,           0.5)\
	X(double,    brain_max_weight_increment,    2.0)\

#define X(TYPE, NAME, VALUE) extern TYPE NAME;
CONFIG_X_MACRO
#undef X

bool load_config(void);
bool save_config(void);
bool verbose(verbosity_t v);


extern const char *default_config_file;

#endif
