/** @file gladiator.c
 *  @brief Using genetic algorithms to select a gladiator controlled by a
 *  neural network. 
 *
 *  @todo make a "headless" mode that does not use the GUI */

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
static prng_t *rstate;
static unsigned generation = 0, tick = 0;
static bool paused = false;
static unsigned alive;

static void keyboard_handler(unsigned char key, int x, int y)
{
	UNUSED(x);
	UNUSED(y);
	key = tolower(key);
	switch(key) {
	case 'p': paused = true;
		  break;
	case 'r': paused = false;
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
	double windowXmin, windowXmax, windowYmin, windowYmax;

	window_width  = w;
	window_height = h;

	glViewport(0, 0, w, h);

	w = (w == 0) ? 1 : w;
	h = (h == 0) ? 1 : h;
	if ((Xmax - Xmin) / w < (Ymax - Ymin) / h) {
		scale = ((Ymax - Ymin) / h) / ((Xmax - Xmin) / w);
		center = (Xmax + Xmin) / 2;
		windowXmin = center - (center - Xmin) * scale;
		windowXmax = center + (Xmax - center) * scale;
		windowYmin = Ymin;
		windowYmax = Ymax;
	} else {
		scale = ((Xmax - Xmin) / w) / ((Ymax - Ymin) / h);
		center = (Ymax + Ymin) / 2;
		windowYmin = center - (center - Ymin) * scale;
		windowYmax = center + (Ymax - center) * scale;
		windowXmin = Xmin;
		windowXmax = Xmax;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(windowXmin, windowXmax, windowYmin, windowYmax, -1, 1);
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
 * work only intermittently.
 *
 * See: https://developer.mozilla.org/en-US/docs/Games/Techniques/2D_collision_detection */
static bool detect_projectile_collision(gladiator_t *g)
{ 
	for(size_t i = 0; i < max_gladiators; i++) {
		if((projectiles[i]->team == g->team) || gladiator_is_dead(g) || !projectile_is_active(projectiles[i]))
			continue;
		double dx = g->x - projectiles[i]->x;
		double dy = g->y - projectiles[i]->y;
		double distance = sqrt(dx * dx + dy * dy);
		if(distance < (projectiles[i]->radius + g->radius)) {
			g->health -= projectile_damage;
			gladiators[i]->hits++;
			if(gladiator_is_dead(gladiators[i])) {
				g->rank = alive--;
			}
			projectile_remove(projectiles[i]);
			return true;
		}
	}
	return false;
}

/**@warning same problems apply as to detect_projectile_collision*/
static bool detect_gladiator_collision(gladiator_t *g)
{ 
	for(size_t i = 0; i < max_gladiators; i++) {
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

/* @todo move to separate file
 * see:
 * https://stackoverflow.com/questions/1073336/circle-line-segment-collision-detection-algorithm*/
static bool detect_line_circle_intersection(
		double Ax, double Ay,
		double Bx, double By,
		double Cx, double Cy, double Cradius)
{
	double lab = sqrt(square(Bx - Ax) + square(By - By));
	double Dx  = (Bx - Ax) / lab;
	double Dy  = (By - Ay) / lab;
	double t   = Dx * (Cx - Ax) + Dy * (Cy - Ay);
	double Ex  = t * Dx + Ax;
	double Ey  = t * Dy + Ay;
	double lec = sqrt(square(Ex - Cx) + square(Ey - Cy));

	if(lec < Cradius) {
		double dt = sqrt(square(Cradius) - square(lec));
		double Fx = (t-dt)*Dx + Ax;
		double Fy = (t-dt)*Dy + Ay;
		double Gx = (t+dt)*Dx + Ax;
		double Gy = (t+dt)*Dy + Ay;
		
		if(verbose(DEBUG)) {
			draw_line(Ax, Ay, 
				PI+atan2((Ay-Cy), (Ax-Cy)), 
				sqrt(square(Ax - Cx) + square(Ay - Cy)), 0.5, MAGENTA);
		}
		draw_regular_polygon_filled(Fx, Fy, 0, 0.5, CIRCLE, CYAN);
		draw_regular_polygon_filled(Gx, Gy, 0, 0.5, CIRCLE, BROWN);
		return true;
	} else if(lec == Cradius) {
		return true;
	} else {
		return false;
	}
}

static bool detect_circle_cone_collision(
		double kx, double ky, double korientation, double ksweep, unsigned lines, double klen,
		double cx, double cy, double cradius)
{
	unsigned j = 0;
	for(double i = korientation - (ksweep/2); j < lines; i += (ksweep/lines), j++) {
		double bx = kx + (klen * cos(i)); /**@note does not handle wrapping */
		double by = ky + (klen * sin(i));
		/*draw_line(bx, by, i, 4, 0.5, WHITE);*/
		bool d = detect_line_circle_intersection(kx, ky, bx, by, cx, cy, cradius);
		if(d)
			return true;
	}
	return false;
}

static bool detect_enemy_gladiator(gladiator_t *k)
{
	for(size_t i = 0; i < max_gladiators; i++) {
		gladiator_t *c = gladiators[i];
		if((k->team == c->team) || (c->health < 0))
			continue;
		bool t = detect_circle_cone_collision(
				k->x, k->y, k->orientation, k->field_of_view, 
				detection_lines, k->radius * gladiator_vision,
				c->x, c->y, c->radius);
		if(t)
			return true;
	}
	return false;
}

static bool detect_enemy_projectile(gladiator_t *k)
{
	for(unsigned j = 0; j < max_gladiators; j++) {
		projectile_t *c = projectiles[j];
		if(k->team == c->team)
			continue;
		bool t = detect_circle_cone_collision(
				k->x, k->y, k->orientation, k->field_of_view, 
				detection_lines, k->radius*gladiator_vision,
				c->x, c->y, c->radius);
		if(t)
			return true;
	}
	return false;

}

static void update_gladiator_inputs(gladiator_t *g, size_t i, double inputs[])
{
	/**@note the order of events really matters, it might be worth
	 * moving them around*/
	inputs[GLADIATOR_IN_FIRED] = projectile_is_active(projectiles[i]);
	inputs[GLADIATOR_IN_FIELD_OF_VIEW] = g->field_of_view;
	inputs[GLADIATOR_IN_VISION_PROJECTILE] = detect_enemy_projectile(g);
	inputs[GLADIATOR_IN_VISION_ENEMY] = detect_enemy_gladiator(g);
	inputs[GLADIATOR_IN_RANDOM]  = prngf(rstate);
	inputs[GLADIATOR_IN_STATE1]  = g->state1;
	inputs[GLADIATOR_IN_COLLISION_ENEMY] = detect_gladiator_collision(g);

	double cval = 0.0;
	cval = g->x == Xmin ? -0.5 : cval;
	cval = g->x == Xmax ? -1.0 : cval;
	cval = g->y == Ymin ? +0.5 : cval;
	cval = g->y == Ymax ? +1.0 : cval;
	if(cval != 0 && verbose(NOTE))
		draw_regular_polygon_filled(g->x, g->y, 0, gladiator_size, CIRCLE, WHITE);

	inputs[GLADIATOR_IN_COLLISION_WALL] = cval;
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


	for(unsigned i = 0; i < max_gladiators; i++)
		detect_projectile_collision(gladiators[i]);
	for(unsigned i = 0; i < max_gladiators; i++)
		projectile_update(projectiles[i]);

	for(unsigned i = 0; i < max_gladiators; i++) {
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
	fill_textbox(WHITE, &t, "alive:      %u/%u", alive, max_gladiators);
	if(verbose(DEBUG))
		fill_textbox(WHITE, &t, "prng:       %f", prngf(rstate));

	for(size_t i = 0; i < max_gladiators; i++) {
		gladiator_t *g = gladiators[i];
		color_t c = team_to_color(g->team);
		fill_textbox(c, &t, "gladiator:  %u", g->team); 
		fill_textbox(c, &t, "health      %f", g->health);
		fill_textbox(c, &t, "angle       %f", g->orientation);
		fill_textbox(c, &t, "hit         %u", g->hits);
		fill_textbox(c, &t, "energy      %f", g->energy);
		fill_textbox(c, &t, "fitness     %f", gladiator_fitness(g));
		fill_textbox(c, &t, "state1      %f", g->state1);
	}

	draw_textbox(GREEN, &t);
}

static void new_generation()
{
	size_t max = 0, min = 0;
	double maxf = 0, minf = 0;
	alive = max_gladiators;
	for(size_t i = 0; i < max_gladiators; i++) {
		double fit = gladiator_fitness(gladiators[i]);
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

	for(size_t i = 0; i < max_gladiators; i++)
		projectile_remove(projectiles[i]);

	for(size_t i = 0; i < max_gladiators; i++)
		gladiator_mutate(gladiators[i]);

	for(size_t i = 0; i < max_gladiators; i++) {
		gladiators[i]->health = gladiator_health;
		gladiators[i]->energy = projectile_energy_cost;
		gladiators[i]->hits = 0;
		gladiators[i]->team = i;
		gladiators[i]->x = prngf(rstate) * Xmax;
		gladiators[i]->y = prngf(rstate) * Ymax;
		gladiators[i]->orientation = prngf(rstate) * 2 * PI;
	}

}

static void draw_scene(void)
{
	static unsigned next = 0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(size_t i = 0; i < max_gladiators; i++) {
		gladiator_draw(gladiators[i]);
		projectile_draw(projectiles[i]);
	}

	draw_debug_info();
	draw_regular_polygon_line(Xmax/2, Ymax/2, PI/4, sqrt(Ymax*Ymax/2), SQUARE, 0.5, WHITE);

	if(next != tick && !paused) {
		if(tick > max_ticks_per_generation) {
			new_generation();
			tick = 0;
			generation++;
		}
		next = tick;
		update_scene();
	}

	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}

static void initialize_arena()
{
	rstate = new_prng(7);
	alive = max_gladiators;
	gladiators = allocate(sizeof(gladiators[0]) * max_gladiators);
	projectiles = allocate(sizeof(projectiles[0]) * max_gladiators);
	for(size_t i = 0; i < max_gladiators; i++) {
		gladiators[i] = gladiator_new(i, prngf(rstate)*Xmax, prngf(rstate)*Ymax, prngf(rstate)*2*PI);
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
	if(!paused)
		tick++;
	glutTimerFunc(tick_ms, timer_callback, value);
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
\t-v  increase verbosity level\n\
\t-s  save configuration file and exit\n\
\t-h  print this help message and exit\n\
\n\
When running there are a few commands that can issued:\n\
\n\
\t'p' pause the simulation\n\
\t'r' resume the simulation after it is paused\n\
\t'q' quit the simulation\n\
\n\
";
	fputs(help, stderr);
}

int main(int argc, char **argv)
{
	char *glut_argv[] = { argv[0], NULL };
	int glut_argc = 0; 
	int debug_level = 0;

	int i;
	for(i = 1; i < argc && argv[i][0] == '-'; i++)
		switch(argv[i][1]) {
		case '\0': /* stop argument processing */
			goto done; 
		case 'v':
			debug_level++;
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
	if(debug_level)
		verbosity = debug_level;

	initialize_arena();
	glutInit(&glut_argc, glut_argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitWindowPosition(window_x, window_y);
	glutInitWindowSize(window_width, window_height);
	glutCreateWindow("Gladiators");

	initialize_rendering();

	glutKeyboardFunc(keyboard_handler);
	glutSpecialFunc(keyboard_special_handler);
	glutReshapeFunc(resize_window);
	glutDisplayFunc(draw_scene);
	glutTimerFunc(tick_ms, timer_callback, 0);

	glutMainLoop();
	return 0;
}
