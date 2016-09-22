/** @file       main.c
 *  @brief      Gladiators program: Evolve neural network controlled gladiators
 *  @author     Richard Howe (2016)
 *  @license    LGPL v2.1 or Later 
 *              <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html> 
 *  @email      howe.r.j.89@gmail.com
 *  @todo a "headless" mode that does not use the GUI
 *  @todo structures for Cartesian and Polar coordinates
 *  @todo allow the arena to be larger than the screen, and add a minimap
 *  @todo add a player object (gladiator, but human controlled)
 *  @todo comment files, add Doxygen headers and LICENSE 
 *  @todo make "food" - which can replenish energy (populate randomly, or from
 *        kills, or both) */

#include "util.h"
#include "gladiator.h"
#include "projectile.h"
#include "collision.h"
#include "vars.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <GL/glut.h>

static gladiator_t **gladiators;
static projectile_t **projectiles;
static unsigned generation = 0, tick = 0;
static bool step = false;
static unsigned alive;

static void keyboard_handler(unsigned char key, int x, int y)
{
	UNUSED(x);
	UNUSED(y);
	key = tolower(key);
	switch(key) {
	case 'p': arena_paused = true;
		  break;
	case 's': step = true;
		  arena_paused = true;
		  break;
	case 'r': arena_paused = false;
		  step = false;
		  break;
	case 'q':
	case ESC:
		exit(EXIT_SUCCESS);
	default:
		break;
	}
}

static void keyboard_special_handler(int key, int x, int y)
{
	UNUSED(x);
	UNUSED(y);
	UNUSED(key);
}

static void resize_window(int w, int h)
{
	double scale, center;
	double window_x_min, window_x_max, window_y_min, window_y_max;

	window_width  = w;
	window_height = h;

	glViewport(0, 0, w, h);

	w = (w == 0) ? 1 : w;
	h = (h == 0) ? 1 : h;
	if ((Xmax - Xmin) / w < (Ymax - Ymin) / h) {
		scale = ((Ymax - Ymin) / h) / ((Xmax - Xmin) / w);
		center = (Xmax + Xmin) / 2;
		window_x_min = center - (center - Xmin) * scale;
		window_x_max = center + (Xmax - center) * scale;
		window_y_min = Ymin;
		window_y_max = Ymax;
	} else {
		scale = ((Xmax - Xmin) / w) / ((Ymax - Ymin) / h);
		center = (Ymax + Ymin) / 2;
		window_y_min = center - (center - Ymin) * scale;
		window_y_max = center + (Ymax - center) * scale;
		window_x_min = Xmin;
		window_x_max = Xmax;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(window_x_min, window_x_max, window_y_min, window_y_max, -1, 1);
}

static double fps(void)
{
	static unsigned frame = 0, timebase = 0;
	static double fps = 0;
	int time = glutGet(GLUT_ELAPSED_TIME);
	frame++;
	if(time - timebase > 1000) {
		fps = frame*1000.0/(time-timebase);
		timebase = time;
		frame = 0;
	}
	return fps;
}

/**@warning if the gladiators or the projectiles are too fast this scheme will
 * work only intermittently, as the projectile warps past the gladiator. This
 * could be resolved with the line-circle detection algorithm to a certain
 * degree.
 *
 * See: https://developer.mozilla.org/en-US/docs/Games/Techniques/2D_collision_detection */
static bool detect_projectile_collision(gladiator_t *g)
{
	if(gladiator_is_dead(g))
		return false;
	for(size_t i = 0; i < arena_gladiator_count; i++) {
		projectile_t *p = projectiles[i];
		if((p->team == g->team) || !projectile_is_active(p))
			continue;
		bool hit = detect_circle_circle_collision(
				g->x, g->y, g->radius,
				p->x, p->y, p->radius);
		if(hit) {
			g->health -= projectile_damage;
			gladiators[i]->hits++;
			if(gladiator_is_dead(gladiators[i]))
				g->rank = alive--;
			projectile_remove(projectiles[i]);
			return true;
		}
	}
	return false;
}

/**@warning same problems apply as to detect_projectile_collision*/
static bool detect_gladiator_collision(gladiator_t *g)
{ 
	for(size_t i = 0; i < arena_gladiator_count; i++) {
		gladiator_t *enemy = gladiators[i];
		if((enemy->team == g->team) || gladiator_is_dead(enemy))
			continue;
		bool hit = detect_circle_circle_collision(
				g->x, g->y, g->radius,
				enemy->x, enemy->y, enemy->radius);
		if(hit) {
			if(draw_gladiator_collision)
				draw_regular_polygon_filled(g->x, g->y, 0, gladiator_size, CIRCLE, RED);
			return true;
		}
	}
	return false;
}

static bool detect_enemy_gladiator(gladiator_t *k)
{
	assert(k);
	for(size_t i = 0; i < arena_gladiator_count; i++) {
		gladiator_t *c = gladiators[i];
		assert(c);
		if((k->team == c->team) || gladiator_is_dead(c))
			continue;
		bool t = detect_circle_cone_collision(
				k->x, k->y, k->orientation, k->field_of_view, 
				detection_lines, k->radius * gladiator_vision,
				c->x, c->y, c->radius, team_to_color(k->team));
		if(t)
			return true;
	}
	return false;
}

static bool detect_enemy_projectile(gladiator_t *k)
{
	assert(k);
	for(unsigned j = 0; j < arena_gladiator_count; j++) {
		projectile_t *c = projectiles[j];
		assert(c);
		if(k->team == c->team || !projectile_is_active(c))
			continue;
		bool t = detect_circle_cone_collision(
				k->x, k->y, k->orientation, k->field_of_view, 
				detection_lines, k->radius*gladiator_vision,
				c->x, c->y, c->radius, team_to_color(k->team));
		if(t)
			return true;
	}
	return false;
}

static void update_gladiator_inputs(gladiator_t *g, double inputs[])
{
	assert(g && inputs);
	inputs[GLADIATOR_IN_CAN_FIRE]          = g->energy > projectile_energy_cost;
	inputs[GLADIATOR_IN_FIELD_OF_VIEW]     = g->field_of_view;
	inputs[GLADIATOR_IN_VISION_PROJECTILE] = detect_enemy_projectile(g);
	inputs[GLADIATOR_IN_VISION_ENEMY]      = detect_enemy_gladiator(g);
	inputs[GLADIATOR_IN_RANDOM]            = random_float();
	inputs[GLADIATOR_IN_STATE1]            = g->state1;
	inputs[GLADIATOR_IN_COLLISION_ENEMY]   = detect_gladiator_collision(g);

	if(arena_wraps_at_edges == false) {
		double cval = 0.0;
		cval = g->x == Xmin ? -0.5 : cval;
		cval = g->x == Xmax ? -1.0 : cval;
		cval = g->y == Ymin ? +0.5 : cval;
		cval = g->y == Ymax ? +1.0 : cval;
		inputs[GLADIATOR_IN_COLLISION_WALL] = cval;
		if(cval != 0 && draw_gladiator_wall_collision)
			draw_regular_polygon_filled(g->x, g->y, 0, gladiator_size, CIRCLE, WHITE);
	}
}

static void update_gladiator_outputs(gladiator_t *g, size_t i, double outputs[])
{
	/*most outputs are handled in gladiator update*/
	bool fire = outputs[GLADIATOR_OUT_FIRE] > gladiator_fire_threshold;
	if(fire && g->energy >= projectile_energy_cost) {
		g->energy -= projectile_energy_cost;
		projectile_fire(projectiles[i], g->x, g->y, g->orientation);
	}
}

static void update_scene(void)
{
	double inputs[GLADIATOR_IN_LAST_INPUT];
	double outputs[GLADIATOR_OUT_LAST_OUTPUT];

	for(unsigned i = 0; i < arena_gladiator_count; i++)
		detect_projectile_collision(gladiators[i]);
	for(unsigned i = 0; i < arena_gladiator_count; i++)
		projectile_update(projectiles[i]);

	for(unsigned i = 0; i < arena_gladiator_count; i++) {
		gladiator_t *g = gladiators[i];
		if(gladiator_is_dead(g))
			continue;

		memset(inputs,  0, sizeof(inputs));
		memset(outputs, 0, sizeof(outputs));

		update_gladiator_inputs(g, inputs);
		gladiator_update(g, inputs, outputs);
		update_gladiator_outputs(g, i, outputs);
	}
}

static void draw_debug_info()
{
	if(!verbose(NOTE))
		return;
	textbox_t t = { .x = Xmin + Xmax/40, .y = Ymax - Ymax/40, .draw_box = false, .color_text = WHITE };

	fill_textbox(&t, print_generation,        "generation: %u", generation);
	fill_textbox(&t, print_arena_tick,        "tick:       %u", tick);
	fill_textbox(&t, print_fps,               "fps:        %f", fps());
	fill_textbox(&t, print_gladiators_alive,  "alive:      %u/%u", alive, arena_gladiator_count);

	for(size_t i = 0; i < arena_gladiator_count; i++) {
		gladiator_t *g = gladiators[i];
		t.color_text = team_to_color(g->team);
		fill_textbox(&t, print_gladiator_team_number,     "gladiator:  %u", g->team); 
		fill_textbox(&t, print_gladiator_health,          "health      %f", g->health);
		fill_textbox(&t, print_gladiator_hits,            "hit         %u", g->hits);
		fill_textbox(&t, print_gladiator_energy,          "energy      %f", g->energy);
		fill_textbox(&t, print_gladiator_fitness,         "fitness     %f", gladiator_fitness(g));
		fill_textbox(&t, print_gladiator_prev_fitness,    "prev. fit.  %f", g->previous_fitness);
		fill_textbox(&t, print_gladiator_mutations,       "mutations   %u", g->mutations);
		fill_textbox(&t, print_gladiator_total_mutations, "total mut.  %u", g->total_mutations);
		fill_textbox(&t, print_gladiator_orientation,     "angle       %f", g->orientation);
		fill_textbox(&t, print_gladiator_x,               "x           %f", g->x);
		fill_textbox(&t, print_gladiator_y,               "y           %f", g->y);
		fill_textbox(&t, print_gladiator_state1,          "state1      %f", g->state1);

	}

	draw_textbox(&t);
}

static int fitness_compare_function(const void *a, const void *b)
{
	gladiator_t *ap = *((gladiator_t**)a);
	gladiator_t *bp = *((gladiator_t**)b);
	if(ap->previous_fitness <  bp->previous_fitness) return -1;
	if(ap->previous_fitness == bp->previous_fitness) return  0;
	if(ap->previous_fitness >  bp->previous_fitness) return  1;
	return 0;
}

static void new_generation()
{
	alive = arena_gladiator_count;
	for(size_t i = 0; i < arena_gladiator_count; i++)
		gladiators[i]->previous_fitness = gladiator_fitness(gladiators[i]);

	qsort(gladiators, arena_gladiator_count, sizeof(gladiators[0]), fitness_compare_function);

	gladiator_t *strongest = gladiators[arena_gladiator_count - 1];
	gladiator_t *weakest   = gladiators[0];
	if(strongest->previous_fitness != weakest->previous_fitness) {
		gladiator_delete(weakest);
		gladiators[0] = gladiator_copy(strongest);
	}

	for(size_t i = 0; i < arena_gladiator_count; i++)
		projectile_remove(projectiles[i]);

	for(size_t i = 0; i < arena_gladiator_count; i++) {
		gladiators[i]->mutations = gladiator_mutate(gladiators[i]);
		gladiators[i]->total_mutations += gladiators[i]->mutations;
	}

	for(size_t i = 0; i < arena_gladiator_count; i++) {
		gladiators[i]->health = gladiator_health;
		gladiators[i]->energy = projectile_energy_cost;
		gladiators[i]->hits = 0;
		gladiators[i]->team = i;
		gladiators[i]->x = random_float() * Xmax;
		gladiators[i]->y = random_float() * Ymax;
		gladiators[i]->orientation = random_float() * 2.0 * PI;
	}
}

static void draw_scene(void)
{
	static unsigned next = 0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(size_t i = 0; i < arena_gladiator_count; i++) {
		gladiator_draw(gladiators[i]);
		projectile_draw(projectiles[i]);
	}

	draw_debug_info();
	draw_regular_polygon_line(Xmax/2, Ymax/2, PI/4, sqrt(Ymax*Ymax/2), SQUARE, 0.5, WHITE);

	if(next != tick && (!arena_paused || step)) {
		if(tick > max_ticks_per_generation) {
			new_generation();
			tick = 0;
			generation++;
		}
		step = false;
		next = tick;
		update_scene();
	}

	textbox_t t = { .x = Xmax/2, .y = Ymax/2, .draw_box = false, .color_text = WHITE };
	fill_textbox(&t, arena_paused, "PAUSED: PRESS 'R' TO CONTINUE");
	fill_textbox(&t, arena_paused, "        PRESS 'S' TO SINGLE STEP");

	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}

static void initialize_arena()
{
	alive = arena_gladiator_count;
	gladiators = allocate(sizeof(gladiators[0]) * arena_gladiator_count);
	projectiles = allocate(sizeof(projectiles[0]) * arena_gladiator_count);
	for(size_t i = 0; i < arena_gladiator_count; i++) {
		/**@todo add in a non random mode */
		double x = random_float()*Xmax;
		double y = random_float()*Ymax;
		double o = random_float()*2*PI;
		gladiators[i] = gladiator_new(i, x, y, o);
		projectiles[i] = projectile_new(i, 0, 0, 0);
	}
}

static void timer_callback(int value)
{
	if(!arena_paused || step)
		tick++;
	glutTimerFunc(arena_tick_ms, timer_callback, value);
}

static void initialize_rendering(char *arg_0)
{
	char *glut_argv[] = { arg_0, NULL };
	int glut_argc = 0;
	glutInit(&glut_argc, glut_argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitWindowPosition(window_x_starting_position, window_y_starting_position);
	glutInitWindowSize(window_width, window_height);
	glutCreateWindow("Gladiators");
	glShadeModel(GL_FLAT);   
	glEnable(GL_DEPTH_TEST);
	glutKeyboardFunc(keyboard_handler);
	glutSpecialFunc(keyboard_special_handler);
	glutReshapeFunc(resize_window);
	glutDisplayFunc(draw_scene);
	glutTimerFunc(arena_tick_ms, timer_callback, 0);
}

static void print_fitness(FILE *out, gladiator_t **g, size_t count)
{
	for(size_t i = 0; i < count; i++)
		fprintf(out, "%.2f ", g[i]->previous_fitness);
	fputc('\n', out);
}

static void headless_loop(FILE *out, unsigned count, bool forever)
{
	for(unsigned tick = 0; generation < count || forever; tick++) { /**@todo allow user to issues commands?*/
		if(tick > max_ticks_per_generation) {
			fprintf(out, "generation: %u\nfitness:    ", generation);
			print_fitness(out, gladiators, arena_gladiator_count);
			new_generation();
			fprintf(out, "            ");
			print_fitness(out, gladiators, arena_gladiator_count);
			tick = 0;
			generation++;
		}
		update_scene();
	}
}

void usage(const char *arg_0)
{
	fprintf(stderr, "usage: %s [-v] [-h] [-]\n", arg_0);
}

void help(void)
{
	static const char help[] = "\
gladiators: fight and evolve a neural network controlled gladiator\n\
\n\
This is a simple toy program designed to display a series of 'gladiators'\n\
that can fire at and evade each other.\n\
\n\
\t-   stop processing options\n\
\t-v  increase verbosity level, overrides configuration\n\
\t-s  save the default configuration file and exit\n\
\t-p  print out the default configuration to stdout and exit\n\
\t-h  print this help message and exit\n\
\n\
When running there are a few commands that can issued:\n\
\n\
\t'p' pause the simulation\n\
\t'r' resume the simulation after it is paused\n\
\t's' run the simulation for a single tick\n\
\t'q' quit the simulation\n\
\n\
";
	fputs(help, stderr);
}

int main(int argc, char **argv)
{

	bool log_level_set = false;
	int log_level = program_log_level;

	int i;
	for(i = 1; i < argc && argv[i][0] == '-'; i++)
		switch(argv[i][1]) {
		case '\0': /* stop argument processing */
			goto done; 
		case 'v':
			log_level++;
			log_level_set = true;
			break;
		case 's':
			fprintf(stderr, "saving configuration file to '%s'\n", default_config_file);
			return !save_config_to_default_config_file();
		case 'p':
			return save_config(stdout);
		case 'h': 
			usage(argv[0]); 
			help();
			return EXIT_SUCCESS; 
		default:
			error("invalid argument '%s'", argv[i][1]);
		}
done:
	load_config();

	if(log_level_set)
		program_log_level = log_level;

	initialize_arena();

	if(program_run_headless) {
		headless_loop(stdout, 0, true);
	} else {
		initialize_rendering(argv[0]);
		glutMainLoop();
	}
	return 0;
}
