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

#define ZERO (0.0)
#define EINS (1.0)
#define SMOL (0.0001)
#define BIGS (10e6)
#define NEGT (-BIGS)

#define CONFIG_X_MACRO\
	X(bool,      world_save_at_exit,                 true,    ZERO,   EINS, "Attempt to save the world state at exit")\
	X(bool,      world_load_at_start,                true,    ZERO,   EINS, "Attempt to load the world state at startup")\
	X(unsigned,  arena_food_count,                   4,       EINS,   BIGS, "The number of food objects in an arena at any given time")\
	X(unsigned,  arena_gladiator_count,              2,       2.0,    BIGS, "The number of gladiators in an arena at in a match")\
	X(unsigned,  arena_gladiator_rounds,             6,       EINS,   BIGS, "The number of gladiator rounds")\
	X(unsigned,  arena_projectile_count,             50,      2.0,    BIGS, "Maximum number of projectiles available to be fired")\
	X(bool,      arena_paused,                       false,   ZERO,   EINS, "Is the arena currently paused, used when displaying the arena and not in headless mode")\
	X(bool,      arena_random_gladiator_start,       true,    ZERO,   EINS, "Is the starting position of each gladiator randomized, or do they start in a circle")\
	X(double,    arena_tick_ms,                      15.0,    ZERO,   BIGS, "Tick speed in milliseconds when in GUI mode")\
	X(bool,      arena_wraps_at_edges,               false,   ZERO,   EINS, "Does the arena wrap at the edges (wrapping is experimental)")\
	X(unsigned,  brain_activation_function,          0,       ZERO,   6.0,  "Activation function for the neurons (0 = logistic, 1 = tanh, 2 = atan, 3 = identity, 4 = step, 5 = rectifier, 6 = sin)")\
	X(unsigned,  brain_input_normalization_method,   1,       ZERO,   2.0,  "Input normalization method to neural network (0 = 0 to 1, 1 = -1 to 1, 2 = -1 OR 1)")\
	X(double,    brain_max_weight_increment,         8.0,     NEGT,   BIGS, "Maximum weight increment per mutation for each neuron weight")\
	X(bool,      brain_mix_in_feedback,              true,    ZERO,   EINS, "Mix the output of the previous neural network run with the current input")\
	X(bool,      brain_retro_is_on,                  true,    ZERO,   EINS, "Is retrograde control on?")\
	X(bool,      brain_internal_state_is_on,         false,   ZERO,   EINS, "Maintain an internal state variable within each neuron that contributes to the neurons output")\
	X(bool,      draw_inactive_projectiles,          false,   ZERO,   EINS, "Draw when two gladiators collide on each gladiator")\
	X(bool,      draw_gladiator_collision,           true,    ZERO,   EINS, "Draw when two gladiators collide on each gladiator")\
	X(bool,      draw_gladiator_target_lines,        true,    ZERO,   EINS, "Draw an indication of each gladiators field of view")\
	X(bool,      draw_gladiator_wall_collision,      true,    ZERO,   EINS, "Draw on the gladiator a visual indication that a wall collision has been detected")\
	X(bool,      draw_gladiator_short_stats,         true,    ZERO,   EINS, "Print a stats on the gladiator next to it")\
	X(bool,      draw_circle_arc_debug_line,         false,   ZERO,   EINS, "Draw a line for debugging the circle arc collision detection")\
	X(double,    fitness_weight_ancestors,           0.0,     NEGT,   BIGS, "Fitness weight for this gladiators ancestors")\
	X(double,    fitness_weight_energy,              0.0,     NEGT,   BIGS, "Fitness weight for energy that a gladiator has")\
	X(double,    fitness_weight_food,                0.2,     NEGT,   BIGS, "Fitness weight for food objects that the gladiator has collected")\
	X(double,    fitness_weight_health,              1.5,     NEGT,   BIGS, "Fitness weight for remaining gladiator health")\
	X(double,    fitness_weight_hits,                1.0,     NEGT,   BIGS, "Fitness weight for number of hits scored")\
	X(double,    fitness_weight_fired,               0.00,    NEGT,   BIGS, "Fitness weight for firing a shot")\
	X(double,    fitness_weight_round,               1.000,   NEGT,   BIGS, "Fitness weight for getting into a higher round")\
	X(double,    fitness_weight_wall_time,           0.0,     NEGT,   BIGS, "Fitness weight for time spent hugging the wall in excess of the wall counter")\
	X(double,    fitness_weight_time_alive,          0.000,   NEGT,   BIGS, "Fitness weight for time spent alive")\
	X(bool,      food_active,                        false,   ZERO,   EINS, "Are the food objects active?")\
	X(unsigned,  food_control_method,                1,       ZERO,   1.0,  "Food control method (0 = random walk, 1 = bound, 2 = avoid gladiator (not implemented))")\
	X(double,    food_distance_per_tick,             0.01,    NEGT,   BIGS, "Method the food moves per tick, depending on the food control method")\
	X(double,    food_health,                        0.25,    NEGT,   BIGS, "How much health food provides")\
	X(double,    food_nourishment,                   150.0,   NEGT,   BIGS, "How much energy food provides")\
	X(bool,      food_respawns,                      true,    ZERO,   EINS, "Does the food respawn after it is eaten")\
	X(double,    food_size,                          1.0,     SMOL,   BIGS, "How big is the food?")\
	X(unsigned,  gladiator_bounce_off_walls,         false,   ZERO,   BIGS, "Do the gladiators bounce off of the walls, or not?")\
	X(unsigned,  gladiator_brain_depth,              2,       EINS,   BIGS, "Number of layers in the Artificial Neural Network for each gladiator")\
	X(unsigned,  gladiator_brain_length,             8,       6.0,    BIGS, "Number of neurons per layer in the Artificial Neural Network for each gladiator, must not be less than the number of inputs to the gladiator, even if that input is not active")\
	X(double,    gladiator_distance_per_tick,        1.0,     NEGT,   BIGS, "The amount a gladiator can move per tick")\
	X(double,    gladiator_energy_increment,         1.0,     NEGT,   BIGS, "The amount of energy a gladiator can gain per tick")\
	X(double,    gladiator_field_of_view_divisor,    1.00,    SMOL,   BIGS, "Divisor for field of view changes")\
	X(double,    gladiator_field_of_view_auto_inc,   0.00,    NEGT,   BIGS, "Amount to automatically increase the field of view by each tick")\
	X(double,    gladiator_fire_threshold,           0.90,    NEGT,   BIGS, "Threshold for output activation for firing")\
	X(double,    gladiator_fire_timeout,             0.0,     ZERO,   BIGS, "Time in ticks before a gladiator can fire again after firing")\
	X(double,    gladiator_health,                   2.0,     ZERO,   BIGS, "The gladiators starting health, should be positive including zero")\
	X(double,    gladiator_max_energy,               500.0,   NEGT,   BIGS, "Maximum energy value a gladiator can have")\
	X(double,    gladiator_max_field_of_view,        1.0,     ZERO,   BIGS, "Maximum field of view angle for the gladiator in radians")\
	X(double,    gladiator_min_field_of_view,        0.15,    ZERO,   BIGS, "Minimum field of view angle for the gladiator in radians")\
	X(double,    gladiator_size,                     3.0,     SMOL,   BIGS, "Size of gladiator in units relative to arena size")\
	X(double,    gladiator_starting_energy,          0.0,     NEGT,   BIGS, "Starting energy for each gladiator, energy is required to fire a projectile")\
	X(double,    gladiator_turn_rate_divisor,        1.00,    SMOL,   BIGS, "Gladiator turn rate divisor - angle in radians ")\
	X(double,    gladiator_vision,                   400.0,   SMOL,   BIGS, "Arc length for field of vision cone")\
	X(double,    gladiator_wall_time,                5.0,     ZERO,   BIGS, "Number of ticks gladiator can spend stuck to a wall before its fitness is decremented")\
	X(double,    max_ticks_per_generation,           10000.0, EINS,   BIGS, "Maximum number of ticks in a match between gladiators")\
	X(double,    mutation_rate,                      0.175,   ZERO,   BIGS, "Rate of mutation (not used directly)")\
	X(bool,      print_arena_tick,                   true,    ZERO,   EINS, "Print the current tick count")\
	X(bool,      print_fps,                          true,    ZERO,   EINS, "Print the frame rate in Frames Per Second")\
	X(bool,      print_generation,                   true,    ZERO,   EINS, "Print the current generation")\
	X(bool,      print_round,                        true,    ZERO,   EINS, "Print the round")\
	X(bool,      print_match,                        true,    ZERO,   EINS, "Print the match")\
	X(bool,      print_gladiator_energy,             false,   ZERO,   EINS, "Print how much energy each gladiator has")\
	X(bool,      print_gladiator_fitness,            true,    ZERO,   EINS, "Print how fit each gladiator is")\
	X(bool,      print_gladiator_health,             false,   ZERO,   EINS, "Print how much health each gladiator has")\
	X(bool,      print_gladiator_hits,               false,   ZERO,   EINS, "Print how many times this gladiator has been hit")\
	X(bool,      print_gladiator_mutations,          false,   ZERO,   EINS, "Print total number of mutations a gladiator has gone through")\
	X(bool,      print_gladiator_orientation,        false,   ZERO,   EINS, "Print the gladiators orientation")\
	X(bool,      print_gladiators_alive,             false,   ZERO,   EINS, "Print whether a gladiator is alive or not")\
	X(bool,      print_gladiator_team_number,        false,   ZERO,   EINS, "Print the gladiators team number")\
	X(bool,      print_gladiator_total_mutations,    false,   ZERO,   EINS, "Print the total number of mutations for this gladiator")\
	X(bool,      print_gladiator_x,                  false,   ZERO,   EINS, "Print gladiator X position")\
	X(bool,      print_gladiator_y,                  false,   ZERO,   EINS, "Print gladiator Y position")\
	X(bool,      input_gladiator_proximity,          false,   ZERO,   EINS, "Turn input proximity sensing on for field of view detection")\
	X(bool,      input_gladiator_has_fired,          true,    ZERO,   EINS, "Turn input 'is there an active projectile fired by this gladiator' on")\
	X(bool,      input_gladiator_field_of_view,      true,    ZERO,   EINS, "Turn input 'gladiator field of view on'")\
	X(bool,      input_gladiator_vision_enemy,       true,    ZERO,   EINS, "Turn input 'gladiator can see enemy in field of view' on")\
	X(bool,      input_gladiator_vision_projectile,  true,    ZERO,   EINS, "Turn input 'gladiator can see enemy projectile in field of view' on")\
	X(bool,      input_gladiator_vision_food,        true,    ZERO,   EINS, "Turn input 'gladiator can see food in field of view' on")\
	X(bool,      input_gladiator_can_fire,           true,    ZERO,   EINS, "Turn input 'is gladiator able to fire' on")\
	X(bool,      input_gladiator_hit,                false,   ZERO,   EINS, "Turn input 'is gladiator hit' on")\
	X(bool,      input_gladiator_random,             false,   ZERO,   EINS, "Turn input 'random value' on, this is an entropy source")\
	X(bool,      input_gladiator_x,                  false,   ZERO,   EINS, "Turn input 'x position' on")\
	X(bool,      input_gladiator_y,                  false,   ZERO,   EINS, "Turn input 'y position' on")\
	X(bool,      input_gladiator_orientation,        false,   ZERO,   EINS, "Turn input 'orientation' on")\
	X(bool,      input_gladiator_collision_enemy,    false,   ZERO,   EINS, "Turn input 'collision with enemy' on")\
	X(bool,      input_gladiator_collision_wall,     false,   ZERO,   EINS, "Turn input 'collision with wall' on")\
	X(unsigned,  program_random_method,              0,       ZERO,   EINS, "Set the Pseudo Random Number Generator used (0 = lcg, 1 = xorshift)")\
	X(unsigned,  program_headless_loops,             30,      ZERO,   BIGS, "Number of loops to run the program without launching the GUI")\
	X(unsigned,  program_log_level,                  NOTE,    ZERO,   5.0,  "Set the program log level")\
	X(bool,      program_pause_after_new_generation, false,   ZERO,   EINS, "In GUI mode, pause after a generation has been completed")\
	X(double,    program_random_seed,                7.0,     ZERO,   BIGS, "The program uses a PRNG that is seeded with this value")\
	X(bool,      program_run_headless,               true,    ZERO,   EINS, "Start the program up in headless mode, which executes much faster")\
	X(bool,      program_run_window_after_headless,  true,    ZERO,   EINS, "After running the program in headless mode, launch the GUI mode so you can see the results")\
	X(double,    projectile_damage,                  1.0,     NEGT,   BIGS, "Damage done by each projectile")\
	X(double,    projectile_distance_per_tick,       1.5,     NEGT,   BIGS, "Distance travelled by a projectile per tick")\
	X(double,    projectile_energy_cost,             50.0,    NEGT,   BIGS, "Energy cost required to fire a projectile")\
	X(double,    projectile_range,                   400.0,   ZERO,   BIGS, "Maximum range for each projectile before it despawns")\
	X(double,    projectile_size,                    2.0,     SMOL,   BIGS, "Size of the projectile")\
	X(bool,      player_active,                      false,   ZERO,   EINS, "Is the player active when in GUI mode?")\
	X(double,    player_distance_per_tick,           1.0,     NEGT,   BIGS, "Distance the player can travel per tick")\
	X(double,    player_energy_increment,            1.0,     NEGT,   BIGS, "Amount of energy gained by the player per tick")\
	X(double,    player_health,                      2.0,     ZERO,   BIGS, "The player health")\
	X(double,    player_max_energy,                  8000.0,  NEGT,   BIGS, "Player maximum energy")\
	X(double,    player_size,                        3.0,     SMOL,   BIGS, "Player size")\
	X(double,    player_starting_energy,             0.0,     NEGT,   BIGS, "Amount of energy the player starts with")\
	X(double,    player_turn_rate_divisor,           6.0,     SMOL,   BIGS, "The turn rate divisor for the player - angle in radians")\
	X(double,    player_refire_timeout,              0.0,     ZERO,   BIGS, "Timeout before player can fire again after firing")\
	X(bool,      breeding_on,                        true,    ZERO,   EINS, "Is the breeding mechanism on?")\
	X(double,    breeding_rate,                      0.9,     ZERO,   EINS, "Amount of gladiators of breeding compared to copying (both with mutations)")\
	X(double,    breeding_crossover_rate,            0.5,     ZERO,   EINS, "Crossover point/rate")\
	X(unsigned,  breeding_crossover_method,          1,       ZERO,   3.0,  "Breeding crossover method (0 = off, 1 = cross-over layers, 2 = swap neurons within layer, 3 = random neuron swap)")\
	X(double,    window_height,                      400.0,   EINS,   BIGS, "GUI Window Height")\
	X(double,    window_width,                       400.0,   EINS,   BIGS, "GUI Window Width")\
	X(double,    window_x_starting_position,         60.0,    ZERO,   BIGS, "GUI Window x-position on screen at startup")\
	X(double,    window_y_starting_position,         20.0,    ZERO,   BIGS, "GUI Window y-position on screen at startup")\
	X(double,    Xmax,                               150.0,   EINS,   BIGS, "Arena maximum x value")\
	X(double,    Xmin,                               0.0,     ZERO,   ZERO, "Arena x zero value")\
	X(double,    Ymax,                               150.0,   EINS,   BIGS, "Arena maximum y value")\
	X(double,    Ymin,                               0.0,     ZERO,   ZERO, "Arena y zero value")\


#define X(TYPE, NAME, VALUE, MINIMUM, MAXIMUM, DESCIPTION) extern TYPE NAME;
CONFIG_X_MACRO
#undef X

int config_load(void);
int config_save(FILE *out);
int config_save_to_default_config_file(void);
int config_help(FILE *out);
bool verbose(verbosity_t v);

void error(const char *fmt, ...);
void warning(const char *fmt, ...);
void note(const char *fmt, ...);
void debug(const char *fmt, ...);

cell_t *config_serialize(void);
int config_deserialize(cell_t *c);

extern const char *default_config_file;

#endif
