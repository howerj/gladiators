#include "brain.h"
#include "util.h"
#include "vars.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

/**@todo make the layer system more generic, so more layers can be added at
 * will
 * @todo load and save brains to disk*/

typedef struct {
	size_t weight_count;
	double bias;
	double threshold; /**@note not used at the moment*/
	double weights[];
} neuron_t;

typedef struct {
	size_t length;
	neuron_t **neurons;
	double *outputs;
} layer_t;

struct brain_t {
	size_t length;
	double  *inputs;
	layer_t *layer1;
	layer_t *layer2;
};

static prng_t *rstate;

static double randomer(double original)
{
	if(!rstate) /**@warning not threadsafe*/
		rstate = new_prng(13);
	double r = prngf(rstate) * brain_max_weight_increment;
	if(prngf(rstate) < 0.5)
		r *= -1;
	if(prngf(rstate) < 0.5)
		r += original;
	else
		r -= original;
	return r;
}

static neuron_t *neuron_new(bool rand, size_t length)
{
	neuron_t *n = allocate(sizeof(*n) + sizeof(n->weights[0])*length);
	n->bias      = rand ? randomer(n->bias) : 1.0;
	n->threshold = rand ? randomer(n->threshold) : 1.0;
	n->weight_count = length;
	for(size_t i = 0; i < length; i++)
		n->weights[i] = rand ? randomer(n->threshold) : 1.0; 
	return n;
}

static void neuron_delete(neuron_t *n)
{
	free(n);
}

static layer_t *layer_new(bool rand, size_t length)
{
	layer_t *l = allocate(sizeof(*l));
	l->length  = length;
	l->outputs = allocate(sizeof(l->outputs[0]) * length);
	l->neurons = allocate(sizeof(l->neurons[0]) * length); 
	for(size_t i = 0; i < length; i++)
		l->neurons[i] = neuron_new(rand, length);
	return l;
}

static void layer_delete(layer_t *l)
{
	for(size_t i = 0; i < l->length; i++)
		neuron_delete(l->neurons[i]);
	free(l->neurons);
	free(l->outputs);
	free(l);
}

static void neuron_copy_over(neuron_t *dst, const neuron_t *src)
{
	dst->bias = src->bias;
	dst->weight_count = src->weight_count;
	dst->threshold = src->threshold;
	for(size_t i = 0; i < src->weight_count; i++)
		dst->weights[i] = src->weights[i];
}

static layer_t *layer_copy(layer_t *l)
{
	layer_t *n = layer_new(false, l->length);
	for(size_t i = 0; i < l->length; i++)
		neuron_copy_over(n->neurons[i], l->neurons[i]);
	return n;
}

static double mutation(double original, size_t length)
{
	double rate = prngf(rstate);
	if(rate <= (mutation_rate/length))
		return randomer(original);
	return original;
}

static void neuron_mutate(neuron_t *n)
{
	n->bias = mutation(n->bias, n->weight_count);
	n->threshold = mutation(n->bias, n->weight_count);
	for(size_t i = 0; i < n->weight_count; i++)
		n->weights[i] = mutation(n->weights[i], n->weight_count);
}

static void neuron_print(FILE *output, neuron_t *n)
{
	fprintf(output, "\tlength:  %zu\n", n->weight_count);
	fprintf(output, "\tbias:    %f\n",  n->bias);
	fprintf(output, "\tweights:\n");
	for(size_t i = 0; i < n->weight_count; i++)
		fprintf(output, "\t\t%f\n", n->weights[i]);
}

static void layer_print(FILE *output, unsigned depth, layer_t *layer)
{
	fprintf(output, "\tlayer %d:\n", depth);
	for(size_t i = 0; i < layer->length; i++)
		neuron_print(output, layer->neurons[i]);
}

void brain_print(FILE *output, brain_t *b)
{
	fprintf(output, "brain:\n");
	layer_print(output, 0, b->layer1);
	layer_print(output, 1, b->layer2);
}

brain_t *brain_new(bool rand, size_t length)
{
	brain_t *b   = allocate(sizeof(*b));
	b->length    = length;
	b->inputs    = allocate(sizeof(b->inputs[0]) * length);
	b->layer1    = layer_new(rand, length);
	b->layer2    = layer_new(rand, length);
	if(verbose(DEBUG) && rand)
		brain_print(stdout, b);
	return b;
}

brain_t *brain_copy(const brain_t *b)
{
	brain_t *n = brain_new(false, b->length);
	n->layer1 = layer_copy(b->layer1);
	n->layer2 = layer_copy(b->layer2);
	return n;
}

void brain_delete(brain_t *b)
{
	if(!b)
		return;
	free(b->inputs);
	layer_delete(b->layer1);
	layer_delete(b->layer2);
	free(b);
}

static double calculate_response(neuron_t *n, const double in[], size_t len) 
{    /* see http://www.cs.bham.ac.uk/~jxb/NN/nn.html*/
	double total = n->bias;
	for(size_t i = 0; i < len; i++)
		total += in[i] * n->weights[i];
	return 1.0 / (1.0 + exp(-total));
}

void update_layer(layer_t *l, const double inputs[], size_t in_length)
{
	for(size_t i = 0; i < in_length; i++)
		l->outputs[i] = calculate_response(l->neurons[i], inputs, in_length);
}

void brain_update(brain_t *b, const double inputs[], size_t in_length, double outputs[], size_t out_length)
{
	assert(b->length > in_length && b->length > out_length);
	for(size_t i = 0; i < in_length; i++)
		b->inputs[i] = inputs[i];
	update_layer(b->layer1, b->inputs, in_length);
	update_layer(b->layer2, b->layer1->outputs, b->layer1->length);
	for(size_t i = 0; i < out_length; i++)
		outputs[i] = b->layer2->outputs[i];
}

void layer_mutate(layer_t *l)
{
	for(size_t i = 0; i < l->length; i++)
		neuron_mutate(l->neurons[i]);
}

void brain_mutate(brain_t *b)
{
	layer_mutate(b->layer1);
	layer_mutate(b->layer2);
}

