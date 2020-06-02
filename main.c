/** @file       main.c
 *  @brief      Gladiators program: Evolve neural network controlled gladiators
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com
 *  @todo allow multiple projectiles to be fired and in play at any one time
 *  @todo Wrap up the configuration options in a configuration structure
 *  @todo add a player object, and player led selection, and a networked
 *  version. It would be interesting to make it so the player has the same kind
 *  of inputs and field of view as the gladiators, so they do not have the
 *  advantage of being able to see the full screen.
 *  @warning all code assumes Xmin and Ymin are zero and Ymax and Xmax are
 *  equal...*/

#include "util.h"
#include "gladiator.h"
#include "projectile.h"
#include "collision.h"
#include "food.h"
#include "player.h"
#include "vars.h"
#include "gui.h"
#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <GL/glut.h>
#include <limits.h>

#define PLAYER_TEAM (UINT_MAX)

typedef struct {
	gladiator_t **gs;
	gladiator_t **population;
	projectile_t **ps;
	food_t **fs;
	player_t *player;
	size_t gladiator_count;
	size_t gladiator_rounds;
	size_t projectile_count;
	size_t food_count;
	unsigned generation;
	unsigned alive;

	unsigned round;
	unsigned match;

	bool player_fire;
	bool player_forward;
	bool player_left;
	bool player_right;
} world_t;

world_t *world;

static unsigned tick = 0;
static bool step = false, skip = false;

/**@warning world_serialize and world_deserialize are for testing purposes only,
 * they do not fully work, nor do they cover things such as the configuration
 * file */
cell_t *world_serialize(world_t *w)
{
	assert(w);
	cell_t *gladiators = cons(mksym("gladiators"), nil());
	cell_t *op   = gladiators;
	for(size_t i = 0; i < w->gladiator_count; op = cdr(op), i++)
		setcdr(op, cons(gladiator_serialize(w->gs[i]), nil()));
	cell_t *projectiles = cons(mksym("projectiles"), nil());
	op = projectiles;
	for(size_t i = 0; i < w->projectile_count; op = cdr(op), i++)
		setcdr(op, cons(projectile_serialize(w->ps[i]), nil()));
	cell_t *foods = cons(mksym("foods"), nil());
	op = foods;
	for(size_t i = 0; i < w->food_count; op = cdr(op), i++)
		setcdr(op, cons(food_serialize(w->fs[i]), nil()));
	cell_t *c = printer(
			"world %x %x %x %x"
			"(gladiator-count %d) "
			"(gladiator-rounds %d) "
			"(projectile-count %d) "
			"(food-count %d) "
			"(generation %d) "
			"(alive %d) ", 
			gladiators, projectiles, foods, player_serialize(w->player),
			(intptr_t)(w->gladiator_count),
			(intptr_t)(w->gladiator_rounds),
			(intptr_t)(w->projectile_count),
			(intptr_t)(w->food_count),
			(intptr_t)(w->generation),
			(intptr_t)(w->alive));
	return c;
}

world_t *world_deserialize(cell_t *c)
{
	assert(c);
	world_t *w = allocate(sizeof(*w));
	cell_t *gs = NULL,  *ps = NULL, *fs = NULL, *player = NULL;
	uintptr_t gsc = 0, grnd = 0, psc = 0, fsc = 0, generation = 0, alive = 0;
	int r = scanner(c,
		"world %x %x %x %x"
		"(gladiator-count %d) "
		"(gladiator-rounds %d) "
		"(projectile-count %d) "
		"(food-count %d) "
		"(generation %d) "
		"(alive %d) ",
		&gs, &ps, &fs, &player,
		&gsc, &grnd, &psc, &fsc,
		&generation, &alive);
	if(r < 0) {
		warning("world deserialization failed for <%p>", c);
		return NULL;
	}

	if(gsc)
		w->population = allocate(sizeof(*gs) * gsc * (1 << grnd));
	w->gs = w->population;
	if(psc)
		w->ps = allocate(sizeof(*ps) * psc);
	if(fsc)
		w->fs = allocate(sizeof(*fs) * fsc);
	if(gsc)
		gs = cdr(gs);
	size_t i;
	for(i = 0 ; i < gsc && type(gs) != NIL; i++, gs = cdr(gs)) {
		cell_type_e wt = type(car(gs));
		if(wt != CONS || !(w->gs[i] = gladiator_deserialize(car(gs)))) {
			warning("gladiator deserialization failed");
			goto fail;
		}
	}	
	if(i != gsc)
		goto fail;
	if(psc)
		ps = cdr(ps);
	for(i = 0 ; i < psc && type(ps) != NIL ; i++, ps = cdr(ps)) {
		cell_type_e wt = type(car(ps));
		if(wt != CONS || !(w->ps[i] = projectile_deserialize(car(ps)))) {
			warning("projectile deserialization failed");
			goto fail;
		}
	}
	if(i != psc)
		goto fail;
	if(fsc)
		fs = cdr(fs);
	for(i = 0 ; i < fsc && type(fs) != NIL; i++, fs = cdr(fs)) {
		cell_type_e wt = type(car(fs));
		if(wt != CONS || !(w->fs[i] = food_deserialize(car(fs)))) {
			warning("food deserialization failed");
			goto fail;
		}
	}
	if(i != fsc)
		goto fail;
	if(!(w->player = player_deserialize(player))) {
		warning("player deserialization failed");
		goto fail;
	}
	w->gladiator_count = gsc;
	w->projectile_count = psc;
	w->food_count = fsc;
	w->generation = generation;
	w->alive = alive;
	return w;
fail:
	return NULL;
}

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
	world_t *w = world;
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
	case ' ': w->player_fire = true;
		  break;
	case 'q':
	case ESC:
		exit(EXIT_SUCCESS);
	default:
		break;
	}
}

static void keyboard_special_down_handler(int key, int x, int y)
{
	UNUSED(x);
	UNUSED(y);
	world_t *w = world;
	switch(key) {
	case GLUT_KEY_UP:    w->player_forward = true; break;
	case GLUT_KEY_LEFT:  w->player_left    = true; break;
	case GLUT_KEY_RIGHT: w->player_right   = true; break;
	default:
		break;
	}
}

static void keyboard_special_up_handler(int key, int x, int y)
{
	UNUSED(x);
	UNUSED(y);
	world_t *w = world;
	switch(key) {
	case GLUT_KEY_UP:    w->player_forward = false; break;
	case GLUT_KEY_LEFT:  w->player_left    = false; break;
	case GLUT_KEY_RIGHT: w->player_right   = false; break;
	default:
		break;
	}
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
static bool detect_projectile_collision(world_t *w, gladiator_t *g, bool hits[])
{
	if(gladiator_is_dead(g))
		return false;
	for(size_t i = 0; i < w->projectile_count; i++) {
		projectile_t *p = w->ps[i];
		if((p->team == g->team) || !projectile_is_active(p))
			continue;
		const bool hit = detect_circle_circle_collision(g->x, g->y, g->radius, p->x, p->y, p->radius);
		if(hit) {
			hits[i] = true;
			g->health -= projectile_damage;
			const unsigned pteam = projectile_team(p);
			if (pteam == PLAYER_TEAM)
				w->player->hits++;
			else
				w->population[projectile_team(p)]->hits++;
			projectile_deactivate(w->ps[i]);
			return true;
		}
	}
	return false;
}

static bool detect_food_collision(world_t *w, gladiator_t *g)
{
	if(gladiator_is_dead(g))
		return false;
	for(size_t i = 0; i < w->food_count; i++) {
		food_t *f = w->fs[i];
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

static bool detect_gladiator(world_t *w, gladiator_t *k, bool detect_enemy_only)
{
	assert(k);
	for(size_t i = 0; i < w->gladiator_count; i++) {
		gladiator_t *c = w->gs[i];
		assert(c);
		if((detect_enemy_only && (k->team == c->team)) || gladiator_is_dead(c))
			continue;
		bool t = detect_circle_arc_collision(
				k->x, k->y, k->orientation, k->field_of_view, 
				k->radius * gladiator_vision,
				c->x, c->y, c->radius);
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
		bool t = detect_circle_arc_collision(
				k->x, k->y, k->orientation, k->field_of_view, 
				k->radius*gladiator_vision,
				c->x, c->y, c->radius);
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
		bool t = detect_circle_arc_collision(
				k->x, k->y, k->orientation, k->field_of_view, 
				k->radius*gladiator_vision,
				f->x, f->y, f->radius);
		if(t)
			return true;
	}
	return false;
}

typedef enum {
	NORMALIZATION_UNITY_E,         /**< range of [ 0, 1] */
	NORMALIZATION_SIGNED_UNITY_E,  /**< range of [-1, 1] */
	NORMALIZATION_SIGNED_BINARY_E, /**< if input is non zero, -1, else 1 */
} normalization_method_t;

#define INPUT_MAX (1.0)
static void update_gladiator_inputs(world_t *w, gladiator_t *g, projectile_t *p, double inputs[], bool hit)
{
	assert(w && g && p && inputs);

	/**@note all inputs should be scaled to be in the [0, 1] range */
	if(input_gladiator_hit)               inputs[GLADIATOR_IN_HIT_GLADIATOR]     = hit;
	if(input_gladiator_can_fire)          inputs[GLADIATOR_IN_CAN_FIRE]          = g->energy > projectile_energy_cost && !projectile_is_active(p);
	if(input_gladiator_field_of_view)     inputs[GLADIATOR_IN_FIELD_OF_VIEW]     = g->field_of_view / gladiator_max_field_of_view;
	if(input_gladiator_vision_food)       inputs[GLADIATOR_IN_VISION_FOOD]       = detect_food(w, g);
	if(input_gladiator_vision_projectile) inputs[GLADIATOR_IN_VISION_PROJECTILE] = detect_enemy_projectile(w, g);
	if(input_gladiator_vision_enemy)      inputs[GLADIATOR_IN_VISION_ENEMY]      = detect_gladiator(w, g, true);
	if(input_gladiator_random)            inputs[GLADIATOR_IN_RANDOM]            = random_float();
	if(input_gladiator_x)                 inputs[GLADIATOR_IN_X]                 = g->x / Xmax;
	if(input_gladiator_y)                 inputs[GLADIATOR_IN_Y]                 = g->y / Ymax;
	if(input_gladiator_orientation)       inputs[GLADIATOR_IN_ORIENTATION]       = g->orientation / (2*PI);
	if(input_gladiator_state1)            inputs[GLADIATOR_IN_STATE1]            = g->state1;
	if(input_gladiator_collision_enemy)   inputs[GLADIATOR_IN_COLLISION_ENEMY]   = detect_gladiator_collision(w, g);

	if(!arena_wraps_at_edges && input_gladiator_collision_wall) {
		bool collision = g->x <= Xmin || g->x >= Xmax || g->y <= Ymin || g->y >= Ymax;
		if(input_gladiator_collision_wall) inputs[GLADIATOR_IN_COLLISION_WALL] = collision;

		if(collision && draw_gladiator_wall_collision)
			draw_regular_polygon_filled(g->x, g->y, 0, gladiator_size, CIRCLE, WHITE);
	}

	for(size_t i = 0; i < GLADIATOR_IN_LAST_INPUT; i++) {
		switch(brain_input_normalization_method) {
		case NORMALIZATION_UNITY_E: /* do nothing */
			break;
		case NORMALIZATION_SIGNED_UNITY_E:
			inputs[i] = (inputs[i]*2*INPUT_MAX) - INPUT_MAX;
			break;
		case NORMALIZATION_SIGNED_BINARY_E:
			inputs[i] = inputs[i] ? 1.0 : -1.0;
			break;
		default:
			error("unknown normalization method: %u", brain_input_normalization_method);
		}
	}
}

static projectile_t *find_free_projectile(projectile_t **ps, size_t count)
{
	for(size_t i = 0; i < count; i++)
		if(!projectile_is_active(ps[i]))
			return ps[i];
	return NULL;
}

static void update_gladiator_outputs(gladiator_t *g, world_t *w, double outputs[])
{
	/*most outputs are handled in "gladiator_update()"*/
	bool fire = outputs[GLADIATOR_OUT_FIRE] > gladiator_fire_threshold;
	if(fire && g->energy >= projectile_energy_cost) {
		projectile_t *p = find_free_projectile(w->ps, w->projectile_count);
		if (!p)
			return;
		if(!(g->refire_timeout)) {
			g->energy -= projectile_energy_cost;
			projectile_fire(p, g->team, g->x, g->y, g->orientation, &g->color);
			g->refire_timeout = gladiator_fire_timeout;
		}
	}
}

static void player_fire(player_t *pl, world_t *w, bool fire) {
	if(fire && pl->energy >= projectile_energy_cost) {
		projectile_t *p = find_free_projectile(w->ps, w->projectile_count);
		if (!p)
			return;
		if(!(pl->refire_timeout)) {
			pl->energy -= projectile_energy_cost;
			projectile_fire(p, pl->team, pl->x, pl->y, pl->orientation, RED);
			pl->refire_timeout = player_refire_timeout;
		}
	}
}

static void update_scene(world_t *w)
{
	double inputs[GLADIATOR_IN_LAST_INPUT] = { 0 };
	double outputs[GLADIATOR_OUT_LAST_OUTPUT] = { 0 };
	bool hits[w->projectile_count];
	w->alive = 0;

	if (player_active) {
		player_update(w->player, w->player_fire, w->player_left, w->player_right, w->player_forward);
		player_fire(w->player, w, w->player_fire);
		w->player_fire = false;
	}

	for(unsigned i = 0; i < w->gladiator_count; i++)
		if(!gladiator_is_dead(w->gs[i]))
			w->alive++;
	for(unsigned i = 0; i < w->gladiator_count; i++)
		detect_projectile_collision(w, w->gs[i], hits);
	for(unsigned i = 0; i < w->gladiator_count; i++)
		detect_food_collision(w, w->gs[i]);
	for(unsigned i = 0; i < w->gladiator_count; i++) {
		gladiator_t  *g = w->gs[i];
		projectile_t *p = w->ps[i];
		unsigned hit = hits[i];
		if(gladiator_is_dead(g))
			continue;
		g->time_alive = tick;
		update_gladiator_inputs(w, g, p, inputs, hit);
		gladiator_update(g, inputs, outputs);
		update_gladiator_outputs(g, w, outputs);
	}
	for(unsigned i = 0; i < w->projectile_count; i++)
		projectile_update(w->ps[i]);
	for(unsigned i = 0; i < w->food_count; i++)
		food_update(w->fs[i]);
}

static void draw_debug_info(world_t *w)
{
	if(!verbose(NOTE))
		return;
	textbox_t t = { .x = Xmin + Xmax/40, .y = Ymax - Ymax/40, .draw_box = false, .color_text = *WHITE };

	fill_textbox(&t, print_generation,        "generation: %u", w->generation);
	fill_textbox(&t, print_arena_tick,        "tick:       %u", tick);
	fill_textbox(&t, print_fps,               "fps:        %f", fps());
	fill_textbox(&t, print_gladiators_alive,  "alive:      %u/%u", w->alive, w->gladiator_count);
	fill_textbox(&t, print_round,             "round:      %u", w->round);
	fill_textbox(&t, print_match,             "match:      %u", w->match);

	for(size_t i = 0; i < w->gladiator_count; i++) {
		gladiator_t *g = w->gs[i];
		t.color_text = g->color;
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

	if(player_active) {
		player_t *p = w->player;
		t.color_text = *WHITE;
		fill_textbox(&t, true, "player");
		//fill_textbox(&t, true, "x/y/angle %f/%f/%f", p->x, p->y, p->orientation);
		fill_textbox(&t, true, "energy %f", p->energy);
		fill_textbox(&t, true, "health %f", p->health);
	}

	draw_textbox(&t);
}

static int fitness_compare_function(const void *a, const void *b)
{
	gladiator_t *ap = *((gladiator_t**)a);
	gladiator_t *bp = *((gladiator_t**)b);
	if(ap->fitness >  bp->fitness) return -1;
	if(ap->fitness == bp->fitness) return  0;
	if(ap->fitness <  bp->fitness) return  1;
	return 0;
}

static void update_fitness(gladiator_t **gs, size_t count)
{
	for(size_t i = 0; i < count; i++)
		gs[i]->fitness = gladiator_fitness(gs[i]);
}

static void mutate_gladiators(gladiator_t **gs, size_t count)
{
	for(size_t i = 0; i < count - 1; i++) {
		gs[i]->mutations = gladiator_mutate(gs[i]);
		gs[i]->total_mutations += gs[i]->mutations;
	}
}

static void reinitialize_gladiator_starting_positions(gladiator_t **gs, size_t count)
{
	assert(gs);
	if(arena_random_gladiator_start) {
		for(size_t i = 0; i < count; i++) {
			gs[i]->x = random_x();
			gs[i]->y = random_y();
			gs[i]->orientation = random_angle();
		}
	} else { /* non random start; gladiators facing outwards in a circle */
		double radius = sqrt(Xmax * Ymax) / 4;
		unsigned j = 0;
		for(double i = 0; i < 2.0 * PI; i += (2.0 * PI) / count, j++) {
			double x = (cos(i) * radius) + Xmax / 2;
			double y = (sin(i) * radius) + Ymax / 2;
			gs[j]->x = x;
			gs[j]->y = y;
			gs[j]->orientation = wrap_rad(i);
		}
	}
}

static void reinitialize_gladiators(gladiator_t **gs, size_t count)
{
	assert(gs);
	for(size_t i = 0; i < count; i++) {
		gs[i]->health = gladiator_health;
		gs[i]->energy = gladiator_starting_energy;
		gs[i]->hits = 0;
		gs[i]->team = i;
		gs[i]->foods = 0;
		gs[i]->field_of_view = 0;
		gs[i]->enemy_gladiator_detected = 0;
		gs[i]->enemy_projectile_detected = 0;
		gs[i]->food_detected = 0;
		gs[i]->wall_contact_timer.i = 0;
	}
	reinitialize_gladiator_starting_positions(gs, count);
}

static void print_fitness(FILE *out, gladiator_t **gs, size_t count)
{
	for(size_t i = 0; i < count; i++)
		fprintf(out, "%.2f ", gs[i]->fitness);
	fputc('\n', out);
}

static void normalize_fitness(gladiator_t **gs, size_t count)
{
	double min = FLT_MAX;
	for(size_t i = 0; i < count; i++)
		min = MIN(min, gs[i]->fitness);
	for(size_t i = 0; i < count; i++)
		gs[i]->fitness = gs[i]->fitness - min;
}

static double total_fitness(gladiator_t **gs, size_t count)
{
	double total = 0;
	normalize_fitness(gs, count);
	for(size_t i = 0; i < count; i++)
		total += gs[i]->fitness;
	return total;
}

static void assign_teams(gladiator_t **gs, size_t count)
{
	for(size_t i = 0; i < count; i++)
		gs[i]->team = i;
}

static void sort_gladiators(gladiator_t **gs, size_t count)
{
	qsort(gs, count, sizeof(gs[0]), fitness_compare_function);
	assign_teams(gs, count);
}

static void shuffle_gladiators(gladiator_t **gs, size_t count)
{
	for(size_t i = 0; i < count; i++) {
		size_t a = random_u64() % count, b = random_u64() % count;
		gladiator_t *t = gs[a];
		gs[a] = gs[b];
		gs[b] = t;
	}
}

static size_t spin_wheel(double wheel[], size_t count)
{
	double r = random_float();
	size_t i;
	for(i = 0; i < count; i++)
		if(wheel[i] >= r)
			break;
	assert(i < count);
	return i;
}

static gladiator_t **roulette_wheel_selection(gladiator_t **gs, size_t count)
{
	assert(gs);
	update_fitness(gs, count);
	sort_gladiators(gs, count);
	double total = total_fitness(gs, count);
	double selection[count];
	gladiator_t **new = allocate(sizeof(new[0]) * count);
	memset(selection, 0, sizeof(selection));
	for(size_t i = 0; i < count; i++)
		selection[i] = gs[i]->fitness / total;
	for(size_t i = 1; i < count; i++)
		selection[i] += selection[i-1];
	for(size_t i = 0; i < count; i++) {
		double breed = random_float();
		if(breed > breeding_rate && breeding_on)
			new[i] = gladiator_breed(gs[spin_wheel(selection, count)], gs[spin_wheel(selection, count)]);
		else
			new[i] = gladiator_copy(gs[spin_wheel(selection, count)]);
	}
	shuffle_gladiators(new, count);
	return new;
}

static void reinitialize_foods(food_t **fs, size_t count)
{
	assert(fs);
	for(size_t i = 0; i < count; i++) { /*randomize food positions*/
		fs[i]->x = random_x();
		fs[i]->y = random_y();
	}
}

static void reinitialize_player(player_t *p)
{
	assert(p);
	p->x = Xmax / 2.0;
	p->y = Ymax / 2.0;
}


static void gladiators_delete(gladiator_t **gs, size_t count)
{
	assert(gs);
	for(size_t i = 0; i < count; i++) {
		gladiator_delete(gs[i]);
		gs[i] = NULL;
	}
	free(gs);
}

static void new_generation(FILE *out, world_t *w)
{
	assert(w);
	size_t all = w->gladiator_count * (1uLL << w->gladiator_rounds);
	for(size_t i = 0; i < w->projectile_count; i++) /*disable all projectiles*/
		projectile_deactivate(w->ps[i]);
	w->alive = w->gladiator_count;

	if(w->match >= ((1u << w->round)-1)) { /* next round */
		w->match = 0;
		w->gs = w->population;
		for(size_t i = 0; i < (1u << (w->round - 1)); i++)
			w->gs[i]->round = w->gladiator_rounds - w->round;
		w->round--;
		sort_gladiators(w->population, 1 << w->round);
		if(!(w->round)) { /* next generation */
			w->generation++;
			w->round = w->gladiator_rounds;
			sort_gladiators(w->gs, all);
			if(verbose(NOTE))
				print_fitness(out, w->population, all);
			gladiator_t **new = roulette_wheel_selection(w->population, all);
			gladiators_delete(w->population, all);
			w->population = new;
			w->gs = w->population;

			mutate_gladiators(w->population, all);
			reinitialize_gladiators(w->population, all);
		}
		if(w->round)
			shuffle_gladiators(w->population, 1 << (w->round - 1));
	} else { /* next match */
		w->match++;
		w->gs += w->gladiator_count;
	}
	reinitialize_gladiators(w->population, all);
	reinitialize_foods(w->fs, w->food_count);
	reinitialize_player(w->player);
}

static void draw_scene(void)
{
	static unsigned next = 0;
	world_t *w = world;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(size_t i = 0; i < w->gladiator_count; i++) {
		gladiator_draw(w->gs[i]);
		projectile_draw(w->ps[i]);
	}

	for(size_t i = 0; i < w->food_count; i++)
		food_draw(w->fs[i]);

	draw_debug_info(w);
	draw_regular_polygon_line(Xmax/2, Ymax/2, PI/4, sqrt(Ymax*Ymax/2), SQUARE, 0.5, WHITE);

	player_draw(w->player);

	if(next != tick && (!arena_paused || step)) {
		if(tick > max_ticks_per_generation || w->alive <= 1 || skip) {
			new_generation(stderr, w);
			tick = 0;
			arena_paused = program_pause_after_new_generation;
			skip = false;
		}
		step = false;
		next = tick;
		update_scene(w);
	}

	textbox_t t = { .x = Xmax/2, .y = Ymax/2, .draw_box = false, .color_text = *WHITE };
	fill_textbox(&t, arena_paused, "PAUSED: PRESS 'R' TO CONTINUE");
	fill_textbox(&t, arena_paused, "        PRESS 'S' TO SINGLE STEP");

	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}

static gladiator_t **gladiators_new(size_t count)
{
	gladiator_t **gs = allocate(sizeof(gs[0]) * count);
	for(size_t i = 0; i < count; i++)
		gs[i] = gladiator_new(i, 0, 0, 0);
	reinitialize_gladiator_starting_positions(gs, count);
	return gs;
}

static projectile_t **projectiles_new(size_t count)
{
	projectile_t **ps = allocate(sizeof(ps[0]) * count);
	for(size_t i = 0; i < count; i++)
		ps[i] = projectile_new(-1, 0, 0, 0);
	return ps;
}

static food_t **foods_new(size_t count)
{
	food_t **fs = NULL;
	if(!count)
		return NULL;
	fs = allocate(sizeof(fs[0]) * count);
	for(size_t i = 0; i < count; i++)
		fs[i] = food_new(random_x(), random_y(), random_angle());
	return fs;
}

static world_t *initialize_arena(size_t gladiator_count, size_t rounds, size_t projectile_count, size_t food_count)
{
	world_t *w = allocate(sizeof(*w));
	w->alive = gladiator_count;
	w->gladiator_count = gladiator_count;
	w->projectile_count = projectile_count;
	w->food_count = food_active ? food_count : 0;
	w->round            = rounds;
	w->gladiator_rounds = rounds;
	w->population = gladiators_new(gladiator_count*(1 << rounds));
	w->gs = w->population;
	w->match = 0;
	w->round = rounds;
	w->generation = 0;;
	w->ps = projectiles_new(projectile_count);
	w->fs = foods_new(food_count);
	w->player = player_new(UINT_MAX);
	w->player->x = Xmax / 2.0;
	w->player->y = Ymax / 2.0;
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
	glutSpecialFunc(keyboard_special_down_handler);
	glutSpecialUpFunc(keyboard_special_up_handler);
	glutReshapeFunc(resize_window);
	glutDisplayFunc(draw_scene);
	glutTimerFunc(arena_tick_ms, timer_callback, 0);
}

static void headless_loop(world_t *w, FILE *out, unsigned count, bool forever)
{ 
	for(tick = 0; w->generation < count || forever; tick++) {
		if(tick > max_ticks_per_generation || w->alive <= 1) {
			update_fitness(w->gs, w->gladiator_count);

			if(verbose(NOTE)) {
				fprintf(out, "generation: %u\tround: %u\tmatch: %u\n", w->generation, w->round, w->match);
				fprintf(out, "fitness:    ");
				print_fitness(out, w->gs, w->gladiator_count);
			}

			new_generation(out, w);

			if(verbose(NOTE)) {
				fprintf(out, "            ");
				print_fitness(out, w->gs, w->gladiator_count);
				fprintf(out, "tick: %u\n", tick);
			}

			tick = 0;
		}
		update_scene(w);
	}
}

void validate_config(void) 
{
	if (gladiator_brain_length <= GLADIATOR_IN_LAST_INPUT)
		error("Too small a brain for inputs %u <= %d", gladiator_brain_length, (unsigned)GLADIATOR_IN_LAST_INPUT);
	if (gladiator_brain_length <= GLADIATOR_OUT_LAST_OUTPUT)
		error("Too small a brain for outputs %u <= %d", gladiator_brain_length, (unsigned)GLADIATOR_OUT_LAST_OUTPUT);
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
In headless mode any human players (if enabled) are not present.\n\
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
			note("saving configuration file to '%s'", default_config_file);
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
	validate_config();

	random_seed(program_random_seed);

	if(run_headless)
		program_run_headless = true;
	if(log_level_set)
		program_log_level = log_level;

	world = initialize_arena(arena_gladiator_count, arena_gladiator_rounds, arena_projectile_count, arena_food_count);

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
