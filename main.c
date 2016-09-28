/** @file       main.c
 *  @brief      Gladiators program: Evolve neural network controlled gladiators
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com
 *  @todo structures for Cartesian and Polar coordinates
 *  @todo split this file up
 *  @todo s-expression based configuration file
 *  @todo more debugging information should be acquired (inputs, outputs, brain
 *  simulator - eg. what happens if I set these inputs, etc.)
 *  @todo allow the arena to be larger than the screen, and add a minimap
 *  @todo add a player object, and player led selection, and a networked
 *  version
 *  @todo Compile time options for headless version only
 *  @todo comment files*/

#include "util.h"
#include "gladiator.h"
#include "projectile.h"
#include "collision.h"
#include "food.h"
#include "vars.h"
#include "gui.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <GL/glut.h>

typedef struct {
	gladiator_t **gs;
	projectile_t **ps;
	food_t **fs;
	size_t gladiator_count;
	size_t projectile_count;
	size_t food_count;
	unsigned generation;
	/**@todo allow this to be saved and loaded from disk, along with the
	 * configuration */
} world_t;

world_t *world;

static unsigned tick = 0;
static bool step = false, skip = false;
static unsigned alive;

static double random_x(void)
{
	return random_float() * Xmax;
}

static double random_y(void) 
{ 
	return random_float() * Ymax; 
}

static double random_angle(void)
{
	return random_float() * 2 * PI;
}

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
	case 'n': skip = true;
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
static bool detect_projectile_collision(world_t *world, gladiator_t *g, bool hits[])
{
	if(gladiator_is_dead(g))
		return false;
	for(size_t i = 0; i < world->projectile_count; i++) {
		projectile_t *p = world->ps[i];
		if((p->team == g->team) || !projectile_is_active(p))
			continue;
		bool hit = detect_circle_circle_collision(
				g->x, g->y, g->radius,
				p->x, p->y, p->radius);
		if(hit) {
			hits[i] = true;
			g->health -= projectile_damage;
			world->gs[i]->hits++;
			projectile_deactivate(world->ps[i]);
			return true;
		}
	}
	return false;
}

static bool detect_food_collision(world_t *world, gladiator_t *g)
{
	if(gladiator_is_dead(g))
		return false;
	for(size_t i = 0; i < world->food_count; i++) {
		food_t *f = world->fs[i];
		if(!food_is_active(f))
			continue;
		bool hit = detect_circle_circle_collision(
				g->x, g->y, g->radius,
				f->x, f->y, f->radius);
		if(hit) {
			g->foods++;
			g->energy += food_nourishment;
			g->health += food_health;
			if(food_respawns) {
				f->x = random_x();
				f->y = random_x();
				f->orientation = random_angle();
			} else {
				food_deactive(f);
			}

			return true;
		}
	}
	return false;
}

/**@warning same problems apply as to detect_projectile_collision*/
static bool detect_gladiator_collision(world_t *w, gladiator_t *g)
{ 
	for(size_t i = 0; i < w->gladiator_count; i++) {
		gladiator_t *enemy = w->gs[i];
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

static bool detect_enemy_gladiator(world_t *w, gladiator_t *k)
{
	assert(k);
	for(size_t i = 0; i < w->gladiator_count; i++) {
		gladiator_t *c = w->gs[i];
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

static bool detect_enemy_projectile(world_t *w, gladiator_t *k)
{
	assert(w && k);
	for(unsigned j = 0; j < w->projectile_count; j++) {
		projectile_t *c = w->ps[j];
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

static bool detect_food(world_t *w, gladiator_t *k)
{
	assert(w && k);
	for(unsigned j = 0; j < w->food_count; j++) {
		food_t *f = w->fs[j];
		assert(f);
		if(!food_is_active(f))
			continue;
		bool t = detect_circle_cone_collision(
				k->x, k->y, k->orientation, k->field_of_view, 
				detection_lines, k->radius*gladiator_vision,
				f->x, f->y, f->radius, team_to_color(k->team));
		if(t)
			return true;
	}
	return false;
}

#define INPUT_MAX (2.0)
static void update_gladiator_inputs(world_t *w, gladiator_t *g, double inputs[], bool hit)
{
	assert(g && inputs);
	/**@todo these inputs might need changing, especially the vision ones,
	 * a direction could be given, whether the target appears in the left
	 * or the right hand field of view...*/
	inputs[GLADIATOR_IN_HIT_GLADIATOR]     = hit;
	inputs[GLADIATOR_IN_CAN_FIRE]          = g->energy > projectile_energy_cost;
	inputs[GLADIATOR_IN_FIELD_OF_VIEW]     = g->field_of_view / gladiator_max_field_of_view;
	inputs[GLADIATOR_IN_VISION_FOOD]       = detect_food(w, g);
	inputs[GLADIATOR_IN_VISION_PROJECTILE] = detect_enemy_projectile(w, g);
	inputs[GLADIATOR_IN_VISION_ENEMY]      = detect_enemy_gladiator(w, g);
	inputs[GLADIATOR_IN_RANDOM]            = random_float();
	inputs[GLADIATOR_IN_X]                 = g->x / Xmax;
	inputs[GLADIATOR_IN_Y]                 = g->y / Ymax;
	inputs[GLADIATOR_IN_ORIENTATION]       = g->orientation / (2*PI);
	inputs[GLADIATOR_IN_STATE1]            = g->state1;
	inputs[GLADIATOR_IN_COLLISION_ENEMY]   = detect_gladiator_collision(w, g);

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

	/* normalizing inputs to [-1, 1] range */
	if(brain_normalize_inputs)
		for(size_t i = 0; i < GLADIATOR_IN_LAST_INPUT; i++)
			inputs[i] = (inputs[i]*INPUT_MAX) - (INPUT_MAX/2.0);
			//inputs[i] = inputs[i] ? 1.0 : -1.0;
}

static void update_gladiator_outputs(gladiator_t *g, projectile_t *p, double outputs[])
{
	/*most outputs are handled in gladiator update*/
	bool fire = outputs[GLADIATOR_OUT_FIRE] > gladiator_fire_threshold;
	if(fire && g->energy >= projectile_energy_cost) {
		g->energy -= projectile_energy_cost;
		projectile_fire(p, g->x, g->y, g->orientation);
	}
}

static void update_scene(world_t *w)
{
	double inputs[GLADIATOR_IN_LAST_INPUT];
	double outputs[GLADIATOR_OUT_LAST_OUTPUT];
	bool hits[w->projectile_count];
	alive = 0;
	memset(hits, 0, sizeof(hits[0]) * w->projectile_count);
	for(unsigned i = 0; i < w->gladiator_count; i++)
		if(!gladiator_is_dead(world->gs[i]))
			alive++;
	for(unsigned i = 0; i < w->gladiator_count; i++)
		detect_projectile_collision(world, w->gs[i], hits);
	for(unsigned i = 0; i < w->gladiator_count; i++)
		detect_food_collision(world, w->gs[i]);
	for(unsigned i = 0; i < w->projectile_count; i++)
		projectile_update(world->ps[i]);
	for(unsigned i = 0; i < w->food_count; i++)
		food_update(world->fs[i]);
	for(unsigned i = 0; i < w->gladiator_count; i++) {
		gladiator_t *g = w->gs[i];
		unsigned hit = hits[i];
		if(gladiator_is_dead(g))
			continue;
		g->rank = tick;

		memset(inputs,  0, sizeof(inputs));
		memset(outputs, 0, sizeof(outputs));

		update_gladiator_inputs(w, g, inputs, hit);
		gladiator_update(g, inputs, outputs);
		update_gladiator_outputs(g, w->ps[i], outputs);
	}
}

static void draw_debug_info(world_t *w)
{
	if(!verbose(NOTE))
		return;
	textbox_t t = { .x = Xmin + Xmax/40, .y = Ymax - Ymax/40, .draw_box = false, .color_text = WHITE };

	fill_textbox(&t, print_generation,        "generation: %u", w->generation);
	fill_textbox(&t, print_arena_tick,        "tick:       %u", tick);
	fill_textbox(&t, print_fps,               "fps:        %f", fps());
	fill_textbox(&t, print_gladiators_alive,  "alive:      %u/%u", alive, w->gladiator_count);

	for(size_t i = 0; i < w->gladiator_count; i++) {
		gladiator_t *g = w->gs[i];
		t.color_text = team_to_color(g->team);
		fill_textbox(&t, print_gladiator_team_number,     "gladiator:  %u", g->team); 
		fill_textbox(&t, print_gladiator_health,          "health      %f", g->health);
		fill_textbox(&t, print_gladiator_hits,            "hit         %u", g->hits);
		fill_textbox(&t, print_gladiator_energy,          "energy      %f", g->energy);
		fill_textbox(&t, print_gladiator_fitness,         "fitness     %f", gladiator_fitness(g));
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
	if(ap->fitness <  bp->fitness) return -1;
	if(ap->fitness == bp->fitness) return  0;
	if(ap->fitness >  bp->fitness) return  1;
	return 0;
}

static void update_fitness(gladiator_t **gs, size_t count)
{
	for(size_t i = 0; i < count; i++)
		gs[i]->fitness = gladiator_fitness(gs[i]);
}

/**@note make a tournament based operating mode
 *
 * A tournament based game mode of operation should be made, a population
 * faces off each other member and roulette wheel selection (and optionally
 * some cross over) decides the next generation.*/
static void new_generation(gladiator_t **gs, size_t count)
{
	update_fitness(gs, count);

	qsort(gs, count, sizeof(gs[0]), fitness_compare_function);

	gladiator_t *strongest = gs[count - 1];
	gladiator_t *weakest   = gs[0];
	if(strongest->fitness != weakest->fitness) {
		gladiator_delete(weakest);
		gs[0] = gladiator_copy(strongest);

		if(count > 2 && use_crossover) {
			gladiator_t *beta = gs[count - 2];
			weakest = gs[1];
			gs[1] = gladiator_breed(beta, strongest);
			gladiator_delete(weakest);
		}
	}

	for(size_t i = 0; i < count - 1; i++) {
		gs[i]->mutations = gladiator_mutate(gs[i]);
		gs[i]->total_mutations += gs[i]->mutations;
	}

	for(size_t i = 0; i < count; i++) {
		gs[i]->health = gladiator_health;
		gs[i]->energy = gladiator_starting_energy;
		gs[i]->hits = 0;
		gs[i]->team = i;
		gs[i]->foods = 0;
		gs[i]->x = random_float() * Xmax;
		gs[i]->y = random_float() * Ymax;
		gs[i]->orientation = random_float() * 2.0 * PI;
		gs[i]->enemy_gladiator_detected = 0;
		gs[i]->enemy_projectile_detected = 0;
		gs[i]->food_detected = 0;
		gs[i]->wall_contact_timer.i = 0;
	}
}

static void draw_scene(void)
{
	static unsigned next = 0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(size_t i = 0; i < world->gladiator_count; i++) {
		gladiator_draw(world->gs[i]);
		projectile_draw(world->ps[i]);
	}

	for(size_t i = 0; i < world->food_count; i++)
		food_draw(world->fs[i]);

	draw_debug_info(world);
	draw_regular_polygon_line(Xmax/2, Ymax/2, PI/4, sqrt(Ymax*Ymax/2), SQUARE, 0.5, WHITE);

	if(next != tick && (!arena_paused || step)) {
		if(tick > max_ticks_per_generation || alive <= 1 || skip) {
			for(size_t i = 0; i < world->projectile_count; i++)
				projectile_deactivate(world->ps[i]);
			alive = world->gladiator_count;

			new_generation(world->gs, world->gladiator_count);
			tick = 0;
			world->generation++;
			arena_paused = program_pause_after_new_generation;
			skip = false;
		}
		step = false;
		next = tick;
		update_scene(world);
	}

	textbox_t t = { .x = Xmax/2, .y = Ymax/2, .draw_box = false, .color_text = WHITE };
	fill_textbox(&t, arena_paused, "PAUSED: PRESS 'R' TO CONTINUE");
	fill_textbox(&t, arena_paused, "        PRESS 'S' TO SINGLE STEP");

	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}

static world_t *initialize_arena(size_t gladiator_and_projectile_count, size_t food_count)
{
	alive = gladiator_and_projectile_count;
	world_t *w = allocate(sizeof(*w));
	w->gladiator_count = gladiator_and_projectile_count;
	w->projectile_count = gladiator_and_projectile_count;
	w->food_count = food_active ? food_count : 0;
	w->gs = allocate(sizeof(w->gs[0]) * gladiator_and_projectile_count);
	w->ps = allocate(sizeof(w->ps[0]) * gladiator_and_projectile_count);
	w->fs = allocate(sizeof(w->fs[0]) * food_count);

	if(arena_random_gladiator_start) {
		for(size_t i = 0; i < gladiator_and_projectile_count; i++) {
			w->gs[i] = gladiator_new(i, random_x(), random_y(), random_angle());
			w->ps[i] = projectile_new(i, 0, 0, 0);
		}
	} else { /* non random start; gladiators facing outwards in a circle */
		double radius = sqrt(Xmax * Ymax) / 4;
		unsigned j = 0;
		for(double i = 0; i < 2.0 * PI; i += (2.0 * PI) / gladiator_and_projectile_count, j++) {
			double x = (cos(i) * radius) + Xmax / 2;
			double y = (sin(i) * radius) + Ymax / 2;
			w->gs[j] = gladiator_new(j, x, y, wrap_rad(i ));
			w->ps[j] = projectile_new(j, 0, 0, 0);
		}
	}

	for(size_t i = 0; i < w->food_count; i++)
		w->fs[i] = food_new(random_x(), random_y(), random_angle());
	return w;
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
		fprintf(out, "%.2f ", g[i]->fitness);
	fputc('\n', out);
}

static void headless_loop(world_t *w, FILE *out, unsigned count, bool forever)
{ 
	for(tick = 0; w->generation < count || forever; tick++) {
		if(tick > max_ticks_per_generation || alive <= 1) {
			update_fitness(w->gs, w->gladiator_count);

			fprintf(out, "generation: %u\n", w->generation);
			fprintf(out, "fitness:    ");
			print_fitness(out, w->gs, w->gladiator_count);

			for(size_t i = 0; i < w->projectile_count; i++)
				projectile_deactivate(w->ps[i]);
			alive = w->gladiator_count;

			new_generation(w->gs, w->gladiator_count);

			fprintf(out, "            ");
			print_fitness(out, w->gs, w->gladiator_count);

			tick = 0;
			w->generation++;
		}
		update_scene(w);
	}
}

void usage(const char *arg_0)
{
	fprintf(stderr, "usage: %s [-v] [-h] [-]\n", arg_0);
}

void help(void)
{
	static const char help[] = "\
arena: fight and evolve a neural network controlled gladiator\n\
\n\
This is a simple toy program designed to display a series of 'gladiators'\n\
that can fire at and evade each other.\n\
\n\
\t-   stop processing options\n\
\t-v  increase verbosity level, overrides configuration\n\
\t-s  save the default configuration file and exit\n\
\t-p  print out the default configuration to stdout and exit\n\
\t-h  print this help message and exit\n\
\t-H  run without the GUI, or run in 'headless' mode\n\
\n\
When running in GUI mode there are a few commands that can issued:\n\
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
	bool run_headless = false;

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
		case 'H': 
			run_headless = true;
			break;
		case 'h': 
			usage(argv[0]); 
			help();
			return EXIT_SUCCESS; 
		default:
			error("invalid argument '%c'", argv[i][1]);
		}
done:
	load_config();
	random_seed(program_random_seed);

	if(run_headless)
		program_run_headless = true;
	if(log_level_set)
		program_log_level = log_level;

	world = initialize_arena(arena_gladiator_count, arena_food_count);

	if(program_run_headless) {
		headless_loop(world, stdout, program_headless_loops, program_headless_loops == 0);
		if(program_run_window_after_headless) {
			program_run_headless = false;
			goto gui;
		}
		return 0;
	} else {
gui:
		initialize_rendering(argv[0]);
		glutMainLoop();
	}
	return 0;
}
