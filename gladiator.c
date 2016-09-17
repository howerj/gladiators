#include "gladiator.h"
#include "util.h"
#include "color.h"
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

typedef struct {
	size_t weight_count;
	double bias;
	double threshold; /*not used at the moment*/
	double weights[];
} neuron_t;

static neuron_t *new_neuron(double bias, size_t len)
{
	neuron_t *n = allocate(sizeof(*n) + sizeof(n->weights[0])*len);
	n->bias = bias;
	n->weight_count = len;
	return n;
}

static double calculate(neuron_t *n, double in[], size_t len) 
{    /* see http://www.cs.bham.ac.uk/~jxb/NN/nn.html*/
	assert(n->weight_count == len);
	double total = n->bias;
	for(size_t i; i < len; i++)
		total += in[i] * n->weights[i];
	return 1.0 / (1.0 + exp(-total));
}

color_t team_to_color(unsigned team)
{
	static const color_t colors[] = { RED, GREEN, YELLOW, CYAN, BLUE, MAGENTA };
	if(team >= INVALID_COLOR) {
		warning("gladiator: ran out of team colors %u");
		return MAGENTA;
	}
	return colors[team];
}

void update_gladiator(gladiator_t *g, const double inputs[], double *outputs[])
{
	UNUSED(g);
	UNUSED(inputs);
	UNUSED(outputs);
}

void draw_gladiator(gladiator_t *g)
{
	draw_line(g->x, g->y, g->orientation, GLADIATOR_SIZE*2, GLADIATOR_SIZE, WHITE);
	draw_line(g->x, g->y, g->orientation - g->field_of_view/2, GLADIATOR_SIZE*10, GLADIATOR_SIZE, MAGENTA);
	draw_line(g->x, g->y, g->orientation + g->field_of_view/2, GLADIATOR_SIZE*10, GLADIATOR_SIZE, MAGENTA);
	draw_regular_polygon(g->x, g->y, g->orientation, GLADIATOR_SIZE, PENTAGON, team_to_color(g->team));
}

gladiator_t *new_gladiator(unsigned team, double x, double y, double orientation)
{
	gladiator_t *g = allocate(sizeof(*g));
	g->team = team;
	g->x = x;
	g->y = y;
	g->orientation = orientation;
	g->field_of_view = PI / 3;
	return g;
}

void delete_gladiator(gladiator_t *g)
{
	free(g);
}

