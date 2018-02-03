/** @file       vars.h
 *  @brief      Global configuration items and management
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

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

/**@note A GUI menu system could be created from this list, potentially*/
/**@todo make the control keys configurable, such as "q" to quit */
#define CONFIG_X_MACRO\
	X(unsigned,  arena_food_count,                   4,       false)\
	X(unsigned,  arena_gladiator_count,              2,       false)\
	X(unsigned,  arena_gladiator_rounds,             5,       false)\
	X(unsigned,  arena_projectile_count,             50,      false)\
	X(bool,      arena_paused,                       false,   true)\
	X(bool,      arena_random_gladiator_start,       true,    true)\
	X(double,    arena_tick_ms,                      15.0,    true)\
	X(bool,      arena_wraps_at_edges,               false,   true)\
	X(unsigned,  brain_activation_function,          2,       true)\
	X(unsigned,  brain_input_normalization_method,   1,       true)\
	X(double,    brain_max_weight_increment,         2.0,     false)\
	X(bool,      brain_mix_in_feedback,              true,    true)\
	X(bool,      draw_gladiator_collision,           true,    true)\
	X(bool,      draw_gladiator_target_lines,        true,    true)\
	X(bool,      draw_gladiator_wall_collision,      true,    true)\
	X(bool,      draw_circle_arc_debug_line,         false,   true)\
	X(double,    fitness_weight_energy,              0.0,     true)\
	X(double,    fitness_weight_food,                1.0,     true)\
	X(double,    fitness_weight_health,              1.5,     true)\
	X(double,    fitness_weight_hits,                1.0,     true)\
	X(double,    fitness_weight_round,               5.000,   true)\
	X(double,    fitness_weight_wall_time,          -0.1,     true)\
	X(double,    fitness_weight_time_alive,          0.001,   true)\
	X(bool,      food_active,                        true,    true)\
	X(unsigned,  food_control_method,                1,       true)\
	X(double,    food_distance_per_tick,             0.01,    true)\
	X(double,    food_health,                        0.25,    true)\
	X(double,    food_nourishment,                   150.0,   true)\
	X(bool,      food_respawns,                      true,    true)\
	X(double,    food_size,                          1.0,     false)\
	X(unsigned,  gladiator_bounce_off_walls,         false,   true)\
	X(unsigned,  gladiator_brain_depth,              6,       false)\
	X(unsigned,  gladiator_brain_length,             30,      false)\
	X(double,    gladiator_distance_per_tick,        1.5,     true)\
	X(double,    gladiator_energy_increment,         10.0,    true)\
	X(double,    gladiator_field_of_view_divisor,    10.00,   false)\
	X(double,    gladiator_fire_threshold,           0.90,    true)\
	X(double,    gladiator_fire_timeout,             0.0,     true)\
	X(double,    gladiator_health,                   10.0,    true)\
	X(double,    gladiator_max_energy,               8000.0,  false)\
	X(double,    gladiator_max_field_of_view,        1.0,     true)\
	X(double,    gladiator_min_field_of_view,        0.15,    true)\
	X(double,    gladiator_size,                     3.0,     false)\
	X(double,    gladiator_starting_energy,          0.0,     true)\
	X(double,    gladiator_turn_rate_divisor,        40.0,    false)\
	X(double,    gladiator_vision,                   400.0,   true)\
	X(double,    gladiator_wall_time,                10.0,    true)\
	X(double,    max_ticks_per_generation,           4000.0,  false)\
	X(double,    mutation_rate,                      0.075,   true)\
	X(bool,      print_arena_tick,                   true,    true)\
	X(bool,      print_fps,                          true,    true)\
	X(bool,      print_generation,                   true,    true)\
	X(bool,      print_round,                        true,    true)\
	X(bool,      print_match,                        true,    true)\
	X(bool,      print_gladiator_energy,             true,    true)\
	X(bool,      print_gladiator_fitness,            true,    true)\
	X(bool,      print_gladiator_health,             true,    true)\
	X(bool,      print_gladiator_hits,               true,    true)\
	X(bool,      print_gladiator_inputs,             true,    true)\
	X(bool,      print_gladiator_mutations,          false,   true)\
	X(bool,      print_gladiator_orientation,        false,   true)\
	X(bool,      print_gladiator_outputs,            true,    true)\
	X(bool,      print_gladiators_alive,             true,    true)\
	X(bool,      print_gladiator_state1,             false,   true)\
	X(bool,      print_gladiator_team_number,        false,   true)\
	X(bool,      print_gladiator_total_mutations,    true,    true)\
	X(bool,      print_gladiator_x,                  false,   true)\
	X(bool,      print_gladiator_y,                  false,   true)\
	X(unsigned,  program_headless_loops,             1000,    true)\
	X(unsigned,  program_log_level,                  NOTE,    true)\
	X(bool,      program_pause_after_new_generation, false,   true)\
	X(double,    program_random_seed,                7.0,     true)\
	X(bool,      program_run_headless,               true,    true)\
	X(bool,      program_run_window_after_headless,  true,    true)\
	X(double,    projectile_damage,                  1.0,     true)\
	X(double,    projectile_distance_per_tick,       2.5,     true)\
	X(double,    projectile_energy_cost,             50.0,    true)\
	X(double,    projectile_range,                   400.0,   true)\
	X(double,    projectile_size,                    2.0,     true)\
	X(bool,      player_active,                      false,   true)\
	X(double,    player_distance_per_tick,           1.0,     true)\
	X(double,    player_energy_increment,            1.0,     true)\
	X(double,    player_health,                      2.0,     true)\
	X(double,    player_max_energy,                  8000.0,  false)\
	X(double,    player_size,                        3.0,     false)\
	X(double,    player_starting_energy,             0.0,     true)\
	X(double,    player_turn_rate_divisor,           6.0,     false)\
	X(bool,      breeding_on,                        true,    true)\
	X(bool,      breeding_rate,                      0.5,     true)\
	X(double,    window_height,                      400.0,   true)\
	X(double,    window_width,                       600.0,   true)\
	X(double,    window_x_starting_position,         60.0,    true)\
	X(double,    window_y_starting_position,         20.0,    true)\
	X(double,    Xmax,                               150.0,   false)\
	X(double,    Xmin,                               0.0,     true)\
	X(double,    Ymax,                               150.0,   false)\
	X(double,    Ymin,                               0.0,     true)\

#define X(TYPE, NAME, VALUE, ZERO_ALLOWED) extern TYPE NAME;
CONFIG_X_MACRO
#undef X

void load_config(void);
bool save_config(FILE *out);
bool save_config_to_default_config_file(void);
bool verbose(verbosity_t v);

void error(const char *fmt, ...);
void warning(const char *fmt, ...);
void note(const char *fmt, ...);
void debug(const char *fmt, ...);

extern const char *default_config_file;

#endif
