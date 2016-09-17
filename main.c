/** @file gladiator.c
 *  @brief Using genetic algorithms to select a gladiator controlled by a
 *  neural network. */

#include "util.h"
#include "gladiator.h"
#include "vars.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <GL/glut.h>

/**@todo Turn into configurable items, along with object properties */
#define TICK_MS        (15)
#define WINDOW_X       (60)
#define WINDOW_Y       (20)
#define MAX_GLADIATORS (2)

static bool redraw;
static gladiator_t *gladiators[MAX_GLADIATORS];
static prng_t *rstate;
static unsigned generation = 0, tick = 0;

static void keyboard_handler(unsigned char key, int x, int y)
{
	UNUSED(x);
	UNUSED(y);
	switch(key) {
	case 'q':
	case 'Q':
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

static void draw_debug_info()
{
	if(!debug_mode)
		return;
	/**@todo find out why -20 is required, it should be 0 */
	textbox_t t = { .x = -20, .y = Ymax - Ymax/40, .draw_box = false };
	fill_textbox(&t, "generation: %u", generation);
	fill_textbox(&t, "tick:       %u", tick);
	fill_textbox(&t, "fps:        %f", fps());
	draw_textbox(&t);
}

static void draw_scene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(unsigned i = 0; i < MAX_GLADIATORS; i++)
		draw_gladiator(gladiators[i]);

	draw_debug_info();

	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}

static void initialize_arena()
{
	rstate = new_prng(7);
	for(unsigned i = 0; i < MAX_GLADIATORS; i++)
		gladiators[i] = new_gladiator(i, prngf(rstate)*Xmax, prngf(rstate)*Ymax, prngf(rstate)*2*PI);
}

static void initialize_rendering(void)
{
	glShadeModel(GL_FLAT);   
	glEnable(GL_DEPTH_TEST);
}

static void timer_callback(int value)
{
	redraw = 1;
	tick++;
	glutTimerFunc(TICK_MS, timer_callback, value);
}

int main(int argc, char **argv)
{
	initialize_arena();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitWindowPosition(WINDOW_X, WINDOW_Y);
	glutInitWindowSize(window_width, window_height);
	glutCreateWindow("Gladiators");

	initialize_rendering();

	glutKeyboardFunc(keyboard_handler);
	glutSpecialFunc(keyboard_special_handler);
	glutReshapeFunc(resize_window);
	glutDisplayFunc(draw_scene);
	glutTimerFunc(TICK_MS, timer_callback, 0);

	glutMainLoop();
	return 0;
}
