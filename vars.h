#ifndef VARS_H
#define VARS_H

#include <stdbool.h>
#include <stdio.h>

typedef enum {
	ALL_MESSAGES_OFF,
	ERROR,
	WARNING,
	NOTE,
	DEBUG,
	ALL_MESSAGES_ON,
} verbosity_t;

#define CONFIG_X_MACRO\
	X(unsigned,  arena_gladiator_count,            5,       false)\
	X(bool,      arena_paused,                     false,   true)\
	X(double,    arena_tick_ms,                    15.0,    true)\
	X(bool,      arena_wraps_at_edges,             false,   true)\
	X(double,    brain_max_weight_increment,       2.0,     false)\
	X(double,    detection_lines,                  100.0,   false)\
	X(bool,      draw_gladiator_collision,         true,    true)\
	X(bool,      draw_gladiator_wall_collision,    true,    true)\
	X(bool,      draw_line_circle_collision,       false,   true)\
	X(bool,      draw_line_circle_debug_line,      false,   true)\
	X(double,    fitness_weight_energy,            0.0,     true)\
	X(double,    fitness_weight_health,            1.5,     true)\
	X(double,    fitness_weight_hits,              1.0,     true)\
	X(double,    fitness_weight_rank,              0.5,     true)\
	X(unsigned,  gladiator_brain_depth,            2,       false)\
	X(unsigned,  gladiator_brain_length,           10,      false)\
	X(double,    gladiator_distance_per_tick,      1.0,     true)\
	X(double,    gladiator_energy_increment,       1.0,     false)\
	X(double,    gladiator_field_of_view_divisor,  32.00,   false)\
	X(double,    gladiator_fire_threshold,         0.50,    true)\
	X(double,    gladiator_fire_timeout,           15.0,    true)\
	X(double,    gladiator_health,                 2.0,     false)\
	X(double,    gladiator_max_energy,             8000.0,  false)\
	X(double,    gladiator_max_field_of_view,      1.0,     true)\
	X(double,    gladiator_min_field_of_view,      0.05,    true)\
	X(double,    gladiator_size,                   3.0,     false)\
	X(double,    gladiator_turn_rate_divisor,      12.0,    false)\
	X(double,    gladiator_vision,                 120.0,   true)\
	X(double,    max_ticks_per_generation,         4000.0,  false)\
	X(double,    mutation_rate,                    0.075,   true)\
	X(bool,      print_arena_tick,                 true,    true)\
	X(bool,      print_fps,                        true,    true)\
	X(bool,      print_generation,                 true,    true)\
	X(bool,      print_gladiator_energy,           true,    true)\
	X(bool,      print_gladiator_fitness,          true,    true)\
	X(bool,      print_gladiator_health,           true,    true)\
	X(bool,      print_gladiator_hits,             true,    true)\
	X(bool,      print_gladiator_inputs,           true,    true)\
	X(bool,      print_gladiator_mutations,        false,   true)\
	X(bool,      print_gladiator_orientation,      false,   true)\
	X(bool,      print_gladiator_outputs,          true,    true)\
	X(bool,      print_gladiator_prev_fitness,     true,    true)\
	X(bool,      print_gladiators_alive,           true,    true)\
	X(bool,      print_gladiator_state1,           false,   true)\
	X(bool,      print_gladiator_team_number,      false,   true)\
	X(bool,      print_gladiator_total_mutations,  true,    true)\
	X(bool,      print_gladiator_x,                false,   true)\
	X(bool,      print_gladiator_y,                false,   true)\
	X(unsigned,  program_log_level,                NOTE,    true)\
	X(double,    program_random_seed,              7.0,     true)\
	X(double,    projectile_damage,                1.0,     true)\
	X(double,    projectile_distance_per_tick,     2.0,     true)\
	X(double,    projectile_energy_cost,           150.0,   true)\
	X(double,    projectile_range,                 250.0,   true)\
	X(double,    projectile_size,                  1.0,     true)\
	X(double,    window_height,                    800.0,   true)\
	X(double,    window_width,                     800.0,   true)\
	X(double,    window_x_starting_position,       60.0,    true)\
	X(double,    window_y_starting_position,       20.0,    true)\
	X(double,    Xmax,                             300.0,   false)\
	X(double,    Xmin,                             0.0,     true)\
	X(double,    Ymax,                             300.0,   false)\
	X(double,    Ymin,                             0.0,     true)\

#define X(TYPE, NAME, VALUE, ZERO_ALLOWED) extern TYPE NAME;
CONFIG_X_MACRO
#undef X

void load_config(void);
bool save_config(FILE *out);
bool save_config_to_default_config_file(void);
bool verbose(verbosity_t v);

void error(const char *fmt, ...);
void warning(const char *fmt, ...);

extern const char *default_config_file;

#endif
