/** @file gladiator.c
 *  @brief Using genetic algorithms to select a gladiator controlled by a
 *  neural network. 
 *
 *  @todo make a "headless" mode that does not use the GUI
 *  @todo make structures for Cartesian and Polar coordinates
 *  @todo comment files, add Doxygen headers and LICENSE */

#include "util.h"
#include "gladiator.h"
#include "projectile.h"
#include "vars.h"
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

/**@todo move to separate file*/
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
		if((projectiles[i]->team == g->team) || !projectile_is_active(projectiles[i]))
			continue;
		double dx = g->x - projectiles[i]->x;
		double dy = g->y - projectiles[i]->y;
		double distance = sqrt(dx * dx + dy * dy);
		if(distance < (projectiles[i]->radius + g->radius)) {
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
		if((gladiators[i]->team == g->team) || gladiator_is_dead(gladiators[i]))
			continue;
		double dx = g->x - gladiators[i]->x;
		double dy = g->y - gladiators[i]->y;
		double distance = sqrt(dx * dx + dy * dy);
		if(distance < (gladiators[i]->radius + g->radius)) {
			if(verbose(NOTE))
				draw_regular_polygon_filled(g->x, g->y, 0, gladiator_size, CIRCLE, RED);
			return true;
		}
	}
	return false;
}

static inline double square(double a)
{
	return a*a;
}

typedef enum {
	quadrant_0_0,
	quadrant_0_1,
	quadrant_1_0,
	quadrant_1_1,
} quadrant_e;

static quadrant_e quadrant(double angle)
{
	quadrant_e r = quadrant_0_0;
	if(angle < PI*0.5)
		r = quadrant_0_0;
	else if(angle < PI)
		r = quadrant_0_1;
	else if(angle < PI*1.5)
		r = quadrant_1_0;
	else
		r = quadrant_1_1;
	return r;
}

/* @todo move to separate file
 * see: https://stackoverflow.com/questions/1073336/circle-line-segment-collision-detection-algorithm */
static bool detect_line_circle_intersection(
		double orientation, bool use_orientation,
		double Ax, double Ay,
		double Bx, double By,
		double Cx, double Cy, double Cradius, color_t color)
{
	double lab = sqrt(square(Bx - Ax) + square(By - Ay));
	double Dx  = (Bx - Ax) / lab;
	double Dy  = (By - Ay) / lab;
	double t   = Dx * (Cx - Ax) + Dy * (Cy - Ay);
	double Ex  = t * Dx + Ax;
	double Ey  = t * Dy + Ay;
	double lec = sqrt(square(Ex - Cx) + square(Ey - Cy));

	if(lec < Cradius) { /* two hits on circle */
		double dt = sqrt(square(Cradius) - square(lec));
		double Fx = (t-dt)*Dx + Ax;
		double Fy = (t-dt)*Dy + Ay;
		double Gx = (t+dt)*Dx + Ax;
		double Gy = (t+dt)*Dy + Ay;
		bool rval = true;

		if(use_orientation) 
			switch(quadrant(orientation)) {
			case quadrant_0_0: rval = Ax < Fx && Gx < Bx; break;
			case quadrant_0_1: rval = Fx < Ax && Bx < Gx; break;
			case quadrant_1_0: rval = Fx < Ax && Bx < Gx; break;
			case quadrant_1_1: rval = Ax < Fx && Gx < Bx; break;
			default: error("invalid quadrant");
			}

		if(rval) {
			draw_regular_polygon_filled(Fx, Fy, 0, 1.0, CIRCLE, color);
			draw_regular_polygon_filled(Gx, Gy, 0, 1.0, CIRCLE, color);
		}

		if(verbose(DEBUG) && rval) {
			draw_line(Ax, Ay, 
				orientation,
				sqrt(square(Ax - Cx) + square(Ay - Cy)), 0.5, MAGENTA);

		}
		return rval;
	} else if(lec == Cradius) { /* tangent */
		return false;
	} else { /* no hits */
		return false;
	}
}

static bool detect_circle_cone_collision(
		double kx, double ky, double korientation, double ksweep, unsigned lines, double klen,
		double cx, double cy, double cradius, color_t color)
{
	unsigned j = 0;
	for(double i = korientation - (ksweep/2); j < lines; i += (ksweep/lines), j++) {
		double bx = kx + (klen * cos(i)); /**@note does not handle wrapping */
		double by = ky + (klen * sin(i));
		bool d = detect_line_circle_intersection(i, true, kx, ky, bx, by, cx, cy, cradius, color);
		if(d)
			return true;
	}
	return false;
}

static bool detect_enemy_gladiator(gladiator_t *k)
{
	for(size_t i = 0; i < arena_gladiator_count; i++) {
		gladiator_t *c = gladiators[i];
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
	for(unsigned j = 0; j < arena_gladiator_count; j++) {
		projectile_t *c = projectiles[j];
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

static void update_gladiator_inputs(gladiator_t *g, size_t i, double inputs[])
{
	inputs[GLADIATOR_IN_FIRED]             = projectile_is_active(projectiles[i]);
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
		if(cval != 0 && verbose(NOTE))
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

		update_gladiator_inputs(g, i, inputs);
		gladiator_update(g, inputs, outputs);
		update_gladiator_outputs(g, i, outputs);
	}
}

static void draw_debug_info()
{
	if(!verbose(NOTE))
		return;
	textbox_t t = { .x = Xmin + Xmax/40, .y = Ymax - Ymax/40, .draw_box = false };
	fill_textbox(WHITE, &t, "generation: %u", generation);
	fill_textbox(WHITE, &t, "tick:       %u", tick);
	fill_textbox(WHITE, &t, "fps:        %f", fps());
	fill_textbox(WHITE, &t, "alive:      %u/%u", alive, arena_gladiator_count);
	if(verbose(DEBUG))
		fill_textbox(WHITE, &t, "prng:       %f", random_float());

	for(size_t i = 0; i < arena_gladiator_count; i++) {
		gladiator_t *g = gladiators[i];
		color_t c = team_to_color(g->team);
		fill_textbox(c, &t, "gladiator:  %u", g->team); 
		fill_textbox(c, &t, "health      %f", g->health);
		fill_textbox(c, &t, "hit         %u", g->hits);
		fill_textbox(c, &t, "energy      %f", g->energy);
		fill_textbox(c, &t, "fitness     %f", gladiator_fitness(g));
		fill_textbox(c, &t, "prev. fit.  %f", g->previous_fitness);
		fill_textbox(c, &t, "mutations   %u", g->mutations);
		fill_textbox(c, &t, "total mut.  %u", g->total_mutations);
		if(verbose(DEBUG)) {
			fill_textbox(c, &t, "angle       %f", g->orientation);
			fill_textbox(c, &t, "state1      %f", g->state1);
		}
	}

	draw_textbox(GREEN, &t);
}

static void new_generation()
{
	size_t max = 0, min = 0;
	double maxf = 0, minf = 0;
	alive = arena_gladiator_count;
	for(size_t i = 0; i < arena_gladiator_count; i++) {
		double fit = gladiator_fitness(gladiators[i]);
		gladiators[i]->previous_fitness = fit;
		if(fit > maxf) {
			max = i;
			maxf = fit;
		}
		if(fit < minf) {
			min = i;
			minf = fit;
		}
	}
	if(minf < maxf) {
		gladiator_t *strongest = gladiator_copy(gladiators[max]);
		gladiator_delete(gladiators[min]);
		gladiators[min] = strongest;
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

	if(arena_paused) {
		textbox_t t = { .x = Xmax/2, .y = Ymax/2, .draw_box = false };
		fill_textbox(WHITE, &t, "PAUSED: PRESS 'R' TO CONTINUE");
		fill_textbox(WHITE, &t, "        PRESS 'S' TO SINGLE STEP");
	}

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

static void initialize_rendering(void)
{
	glShadeModel(GL_FLAT);   
	glEnable(GL_DEPTH_TEST);
}

static void timer_callback(int value)
{
	if(!arena_paused || step)
		tick++;
	glutTimerFunc(arena_tick_ms, timer_callback, value);
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
\t-s  save configuration file and exit\n\
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
	char *glut_argv[] = { argv[0], NULL };
	int glut_argc = 0;

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
			return !save_config();
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
	glutInit(&glut_argc, glut_argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitWindowPosition(window_x_starting_position, window_y_starting_position);
	glutInitWindowSize(window_width, window_height);
	glutCreateWindow("Gladiators");

	initialize_rendering();

	glutKeyboardFunc(keyboard_handler);
	glutSpecialFunc(keyboard_special_handler);
	glutReshapeFunc(resize_window);
	glutDisplayFunc(draw_scene);
	glutTimerFunc(arena_tick_ms, timer_callback, 0);

	glutMainLoop();
	return 0;
}
