/** @file       vars.h
 *  @brief      Global configuration items and management
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com*/

#ifndef VARS_H
#define VARS_H

#include <stdbool.h>
#include <stdio.h>
#include "sexpr.h"

typedef enum {
	ALL_MESSAGES_OFF,
	ERROR,
	WARNING,
	NOTE,
	DEBUG,
	ALL_MESSAGES_ON,
} verbosity_t;

/* TODO: Use min/max values and remove 'cannot be zero flag', also, strings! */
#define CONFIG_X_MACRO\
	X(bool,      world_save_at_exit,                 true,    false,  "Attempt to save the world state at exit")\
	X(bool,      world_load_at_start,                true,    false,  "Attempt to load the world state at startup")\
	X(unsigned,  arena_food_count,                   4,       false,  "The number of food objects in an arena at any given time")\
	X(unsigned,  arena_gladiator_count,              2,       false,  "The number of gladiators in an arena at in a match")\
	X(unsigned,  arena_gladiator_rounds,             6,       false,  "The number of gladiator rounds")\
	X(unsigned,  arena_projectile_count,             50,      false,  "Maximum number of projectiles available to be fired")\
	X(bool,      arena_paused,                       false,   true,   "Is the arena currently paused, used when displaying the arena and not in headless mode")\
	X(bool,      arena_random_gladiator_start,       true,    true,   "Is the starting position of each gladiator randomized, or do they start in a circle")\
	X(double,    arena_tick_ms,                      15.0,    true,   "Tick speed in milliseconds when in GUI mode")\
	X(bool,      arena_wraps_at_edges,               false,   true,   "Does the arena wrap at the edges (wrapping is experimental)")\
	X(unsigned,  brain_activation_function,          2,       true,   "Activation function for the neurons (0 = logistic, 1 = tanh, 2 = atan, 3 = identity, 4 = step, 5 = rectifier, 6 = sin)")\
	X(unsigned,  brain_input_normalization_method,   1,       true,   "Input normalization method to neural network (0 = 0 to 1, 1 = -1 to 1, 2 = -1 OR 1)")\
	X(double,    brain_max_weight_increment,         2.0,     false,  "Maximum weight increment per mutation for each neuron weight")\
	X(bool,      brain_mix_in_feedback,              true,    true,   "Mix the output of the previous neural network run with the current input")\
	X(bool,      brain_internal_state_is_on,         false,   true,   "Maintain an internal state variable within each neuron that contributes to the neurons output")\
	X(bool,      draw_gladiator_collision,           true,    true,   "Draw when two gladiators collide on each gladiator")\
	X(bool,      draw_gladiator_target_lines,        true,    true,   "Draw an indication of each gladiators field of view")\
	X(bool,      draw_gladiator_wall_collision,      true,    true,   "Draw on the gladiator a visual indication that a wall collision has been detected")\
	X(bool,      draw_gladiator_short_stats,         true,    true,   "Print a stats on the gladiator next to it")\
	X(bool,      draw_circle_arc_debug_line,         false,   true,   "Draw a line for debugging the circle arc collision detection")\
	X(double,    fitness_weight_ancestors,           0.0,     true,   "Fitness weight for this gladiators ancestors")\
	X(double,    fitness_weight_energy,              0.0,     true,   "Fitness weight for energy that a gladiator has")\
	X(double,    fitness_weight_food,                0.2,     true,   "Fitness weight for food objects that the gladiator has collected")\
	X(double,    fitness_weight_health,              1.5,     true,   "Fitness weight for remaining gladiator health")\
	X(double,    fitness_weight_hits,                1.0,     true,   "Fitness weight for number of hits scored")\
	X(double,    fitness_weight_round,               5.000,   true,   "Fitness weight for getting into a higher round")\
	X(double,    fitness_weight_wall_time,          -0.3,     true,   "Fitness weight for time spent hugging the wall in excess of the wall counter")\
	X(double,    fitness_weight_time_alive,          0.000,   true,   "Fitness weight for time spend alive")\
	X(bool,      food_active,                        false,   true,   "Are the food objects active?")\
	X(unsigned,  food_control_method,                1,       true,   "Food control method (0 = random walk, 1 = bound)")\
	X(double,    food_distance_per_tick,             0.01,    true,   "Method the food moves per tick, depending on the food control method")\
	X(double,    food_health,                        0.25,    true,   "How much health food provides")\
	X(double,    food_nourishment,                   150.0,   true,   "How much energy food provides")\
	X(bool,      food_respawns,                      true,    true,   "Does the food respawn after it is eaten")\
	X(double,    food_size,                          1.0,     false,  "How big is the food?")\
	X(unsigned,  gladiator_bounce_off_walls,         false,   true,   "Do the gladiators bounce off of the walls, or not?")\
	X(unsigned,  gladiator_brain_depth,              5,       false,  "Number of layers in the Artificial Neural Network for each gladiator")\
	X(unsigned,  gladiator_brain_length,             25,      false,  "Number of neurons per layer in the Artificial Neural Network for each gladiator, must not be less than the number of inputs to the gladiator, even if that input is not active")\
	X(double,    gladiator_distance_per_tick,        1.0,     true,   "The amount a gladiator can move per tick")\
	X(double,    gladiator_energy_increment,         1.0,     true,   "The amount of energy a gladiator can gain per tick")\
	X(double,    gladiator_field_of_view_divisor,    10.00,   false,  "")\
	X(double,    gladiator_field_of_view_auto_inc,   0.00,    true,   "Amount to automatically increase the field of view by each tick")\
	X(double,    gladiator_fire_threshold,           0.90,    true,   "Threshold for output activation for firing")\
	X(double,    gladiator_fire_timeout,             0.0,     true,   "Time in ticks before a gladiator can fire again after firing")\
	X(double,    gladiator_health,                   2.0,     true,   "The gladiators starting health, should be positive including zero")\
	X(double,    gladiator_max_energy,               8000.0,  false,  "Maximum energy value a gladiator can have")\
	X(double,    gladiator_max_field_of_view,        1.0,     true,   "Maximum field of view angle for the gladiator in radians")\
	X(double,    gladiator_min_field_of_view,        0.15,    true,   "Minimum field of view angle for the gladiator in radians")\
	X(double,    gladiator_size,                     3.0,     false,  "Size of gladiator in units relative to arena size")\
	X(double,    gladiator_starting_energy,          0.0,     true,   "Starting energy for each gladiator, energy is required to fire a projectile")\
	X(double,    gladiator_turn_rate_divisor,        7.30,    false,  "")\
	X(double,    gladiator_vision,                   400.0,   true,   "Arc length for field of vision cone")\
	X(double,    gladiator_wall_time,                5.0,     true,   "")\
	X(double,    max_ticks_per_generation,           10000.0, false,  "Maximum number of ticks in a match between gladiators")\
	X(double,    mutation_rate,                      0.175,   true,   "")\
	X(bool,      print_arena_tick,                   true,    true,   "Print the current tick count")\
	X(bool,      print_fps,                          true,    true,   "Print the frame rate in Frames Per Second")\
	X(bool,      print_generation,                   true,    true,   "Print the current generation")\
	X(bool,      print_round,                        true,    true,   "Print the round")\
	X(bool,      print_match,                        true,    true,   "Print the match")\
	X(bool,      print_gladiator_energy,             false,   true,   "Print how much energy each gladiator has")\
	X(bool,      print_gladiator_fitness,            true,    true,   "Print how fit each gladiator is")\
	X(bool,      print_gladiator_health,             false,   true,   "Print how much health each gladiator has")\
	X(bool,      print_gladiator_hits,               false,   true,   "Print how many times this gladiator has been hit")\
	X(bool,      print_gladiator_mutations,          false,   true,   "Print total number of mutations a gladiator has gone through")\
	X(bool,      print_gladiator_orientation,        false,   true,   "Print the gladiators orientation")\
	X(bool,      print_gladiators_alive,             false,   true,   "Print whether a gladiator is alive or not")\
	X(bool,      print_gladiator_state1,             false,   true,   "Print the current state variable value")\
	X(bool,      print_gladiator_team_number,        false,   true,   "Print the gladiators team number")\
	X(bool,      print_gladiator_total_mutations,    false,   true,   "Print the total number of mutations for this gladiator")\
	X(bool,      print_gladiator_x,                  false,   true,   "Print gladiator X position")\
	X(bool,      print_gladiator_y,                  false,   true,   "Print gladiator Y position")\
	X(bool,      input_gladiator_hit,                true,    true,   "Turn input 'is gladiator hit' on")\
	X(bool,      input_gladiator_can_fire,           true,    true,   "Turn input 'is gladiator able to fire' on")\
	X(bool,      input_gladiator_field_of_view,      true,    true,   "Turn input 'gladiator field of view on'")\
	X(bool,      input_gladiator_vision_food,        true,    true,   "Turn input 'gladiator can see food in field of view' on")\
	X(bool,      input_gladiator_vision_projectile,  true,    true,   "Turn input 'gladiator can see enemy projectile in field of view' on")\
	X(bool,      input_gladiator_vision_enemy,       true,    true,   "Turn input 'gladiator can see enemy in field of view' on")\
	X(bool,      input_gladiator_random,             true,    true,   "Turn input 'random value' on, this is an entropy source")\
	X(bool,      input_gladiator_x,                  true,    true,   "Turn input 'x position' on")\
	X(bool,      input_gladiator_y,                  true,    true,   "Turn input 'y position' on")\
	X(bool,      input_gladiator_orientation,        true,    true,   "Turn input 'orientation' on")\
	X(bool,      input_gladiator_state1,             true,    true,   "Turn input 'state variable' on")\
	X(bool,      input_gladiator_collision_enemy,    true,    true,   "Turn input 'collision with enemy' on")\
	X(bool,      input_gladiator_collision_wall,     true,    true,   "Turn input 'collision with wall' on")\
	X(unsigned,  program_headless_loops,             30,      true,   "")\
	X(unsigned,  program_log_level,                  NOTE,    true,   "Set the program log level")\
	X(bool,      program_pause_after_new_generation, false,   true,   "In GUI mode, pause after a generation has been completed")\
	X(double,    program_random_seed,                7.0,     true,   "The program uses a PRNG that is seeded with this value")\
	X(bool,      program_run_headless,               true,    true,   "Start the program up in headless mode, which executes much faster")\
	X(bool,      program_run_window_after_headless,  true,    true,   "After running the program in headless mode, launch the GUI mode so you can see the results")\
	X(double,    projectile_damage,                  1.0,     true,   "Damage done by each projectile")\
	X(double,    projectile_distance_per_tick,       1.5,     true,   "Distance travelled by a projectile per tick")\
	X(double,    projectile_energy_cost,             50.0,    true,   "Energy cost required to fire a projectile")\
	X(double,    projectile_range,                   400.0,   true,   "Maximum range for each projectile before it despawns")\
	X(double,    projectile_size,                    2.0,     true,   "Size of the projectile")\
	X(bool,      player_active,                      false,   true,   "Is the player active when in GUI mode?")\
	X(double,    player_distance_per_tick,           1.0,     true,   "Distance the player can travel per tick")\
	X(double,    player_energy_increment,            1.0,     true,   "Amount of energy gained by the player per tick")\
	X(double,    player_health,                      2.0,     true,   "The player health")\
	X(double,    player_max_energy,                  8000.0,  false,  "Player maximum energy")\
	X(double,    player_size,                        3.0,     false,  "Player size")\
	X(double,    player_starting_energy,             0.0,     true,   "Amount of energy the player starts with")\
	X(double,    player_turn_rate_divisor,           6.0,     false,  "")\
	X(double,    player_refire_timeout,              0.0,     true,   "Timeout before player can fire again after firing")\
	X(bool,      breeding_on,                        true,    true,   "Is the breeding mechanism on?")\
	X(double,    breeding_rate,                      0.9,     true,   "")\
	X(double,    breeding_crossover_rate,            0.5,     true,   "")\
	X(unsigned,  breeding_crossover_method,          1,       true,   "")\
	X(double,    window_height,                      400.0,   true,   "GUI Window Height")\
	X(double,    window_width,                       400.0,   true,   "GUI Window Width")\
	X(double,    window_x_starting_position,         60.0,    true,   "GUI Window x-position on screen at startup")\
	X(double,    window_y_starting_position,         20.0,    true,   "GUI Window y-position on screen at startup")\
	X(double,    Xmax,                               150.0,   false,  "Arena m")\
	X(double,    Xmin,                               0.0,     true,   "")\
	X(double,    Ymax,                               150.0,   false,  "")\
	X(double,    Ymin,                               0.0,     true,   "")\


#define X(TYPE, NAME, VALUE, ZERO_ALLOWED, DESCIPTION) extern TYPE NAME;
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

cell_t *config_serialize(void);
int config_deserialize(cell_t *c);

extern const char *default_config_file;

#endif
