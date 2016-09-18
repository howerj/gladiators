#ifndef VARS_H
#define VARS_H

#include <stdbool.h>

#define CONFIG_X_MACRO\
	X(bool,   debug_mode,                   true)\
	X(double, Xmin,                         0.0)\
	X(double, Xmax,                         150.0)\
	X(double, Ymin,                         0.0)\
	X(double, Ymax,                         150.0)\
	X(double, window_width,                 800.0)\
	X(double, window_height,                500.0)\
	X(double, tick_ms,                      15.0)\
	X(double, max_ticks_per_generation,     500.0)\
	X(double, window_x,                     60.0)\
	X(double, window_y,                     20.0)\
	X(double, detection_lines,              100.0)\
	X(double, mutation_rate,                0.15)\
	X(double, gladiator_size,               3.0)\
	X(double, gladiator_health,             2.0)\
	X(double, gladiator_fire_timeout,       15.0)\
	X(double, gladiator_brain_length,       5.0)\
	X(double, gladiator_distance_per_tick,  1.0)\
	X(double, gladiator_fire_threshold,     0.50)\
	X(double, gladiator_vision,             70.0)\
	X(double, projectile_distance_per_tick, 2.0)\
	X(double, projectile_range,             100)\
	X(double, projectile_size,              1.0)\
	X(double, projectile_damage,            1.0)\
	X(double, projectile_energy_cost,       50.0)\
	X(double, brain_max_weight,             2.0)\
	X(double, max_gladiators,               3.0)\

#define X(TYPE, NAME, VALUE) extern TYPE NAME;
CONFIG_X_MACRO
#undef X

bool load_config(void);
bool save_config(void);

#endif
