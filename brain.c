#include "brain.h"
#include "util.h"
#include "vars.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

typedef struct {
	size_t weight_count;
	double bias;
	double threshold; /*not used at the moment*/
	double weights[];
} neuron_t;

struct brain_t {
	size_t length;
	double   *input;
	neuron_t **layer1;
	double   *layer1out;
	neuron_t **layer2;
	double   *layer2out;
};

neuron_t *new_neuron(prng_t *r, size_t length)
{
	neuron_t *n = allocate(sizeof(*n) + sizeof(n->weights[0])*length);
	n->bias = prngf(r) * BRAIN_MAX_WEIGHT;
	n->weight_count = length;
	for(size_t i = 0; i < length; i++)
		n->weights[i] = prngf(r) * BRAIN_MAX_WEIGHT; 
	return n;
}

static void print_neuron(FILE *output, neuron_t *n)
{
	fprintf(output, "\tlength:  %zu\n", n->weight_count);
	fprintf(output, "\tbias:    %f\n",  n->bias);
	fprintf(output, "\tweights:\n");
	for(size_t i = 0; i < n->weight_count; i++)
		fprintf(output, "\t\t%f\n", n->weights[i]);
}

static void print_layer(FILE *output, size_t length, unsigned depth, neuron_t **layer)
{
	fprintf(output, "\tlayer %d:\n", depth);
	for(size_t i = 0; i < length; i++)
		print_neuron(output, layer[i]);
}

void print_brain(FILE *output, brain_t *b)
{
	fprintf(output, "brain:\n");
	print_layer(output, b->length, 0, b->layer1);
	print_layer(output, b->length, 0, b->layer2);
}

brain_t *new_brain(prng_t *r, size_t length)
{
	brain_t *b   = allocate(sizeof(*b));
	b->input     = allocate(sizeof(b->input[0])    * length);
	b->layer1    = allocate(sizeof(b->layer1[0])   * length);
	b->layer1out = allocate(sizeof(b->layer1out[0])* length);
	b->layer2    = allocate(sizeof(b->layer2[0])   * length);
	b->layer2out = allocate(sizeof(b->layer2out[0])* length);
	for(size_t i = 0; i < length; i++) {
		b->layer1[i] = new_neuron(r, length);
		b->layer2[i] = new_neuron(r, length);
	}
	b->length = length;
	if(debug_mode)
		print_brain(stdout, b);
	return b;
}

void delete_brain(brain_t *b)
{
	if(!b)
		return;
	free(b->input);
	free(b->layer1);
	free(b->layer2);
	free(b);
}

static double calculate_response(neuron_t *n, double in[], size_t len) 
{    /* see http://www.cs.bham.ac.uk/~jxb/NN/nn.html*/
	assert(n->weight_count == len);
	double total = n->bias;
	for(size_t i; i < len; i++)
		total += in[i] * n->weights[i];
	return 1.0 / (1.0 + exp(-total));
}

void update_brain(brain_t *b, const double inputs[], size_t in_length, double outputs[], size_t out_length)
{
	for(size_t i = 0; i < in_length; i++)
		b->input[i] = inputs[i];
	for(size_t i = 0; i < b->length; i++)
		b->layer1out[i] = calculate_response(b->layer1[i], b->input, b->length);
	for(size_t i = 0; i < b->length; i++)
		b->layer2out[i] = calculate_response(b->layer2[i], b->layer1out, b->length);
	for(size_t i = 0; i < out_length; i++)
		outputs[i] = b->layer2out[i];
}

