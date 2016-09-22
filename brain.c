/** @file       brain.c
 *  @brief      A multilayer neural network
 *  @author     Richard Howe (2016)
 *  @license    LGPL v2.1 or Later 
 *              <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html> 
 *  @email      howe.r.j.89@gmail.com
 *  @todo       make calculate_response configurable, so we can use different
 *              functions in its palce*/
#include "brain.h"
#include "util.h"
#include "vars.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/**@todo load and save brains to disk*/

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
	size_t depth;
	double  *inputs;
	layer_t **layers;
};

static double randomer(double original)
{
	double r = random_float() * brain_max_weight_increment;
	if(random_float() < 0.5)
		r *= -1;
	if(random_float() < 0.5)
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
	assert(l);
	for(size_t i = 0; i < l->length; i++)
		neuron_delete(l->neurons[i]);
	free(l->neurons);
	free(l->outputs);
	free(l);
}

static void neuron_copy_over(neuron_t *dst, const neuron_t *src)
{
	assert(dst && src);
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

static double mutation(double original, size_t length, unsigned *count)
{
	assert(count);
	double rate = random_float();
	if(rate <= (mutation_rate/length)) {
		(*count)++;
		return randomer(original);
	}
	return original;
}

static void neuron_mutate(neuron_t *n, unsigned *count)
{
	assert(n && count);
	n->bias = mutation(n->bias, n->weight_count, count);
	n->threshold = mutation(n->bias, n->weight_count, count);
	for(size_t i = 0; i < n->weight_count; i++)
		n->weights[i] = mutation(n->weights[i], n->weight_count, count);
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
	assert(output && layer);
	fprintf(output, "\tlayer %d:\n", depth);
	for(size_t i = 0; i < layer->length; i++)
		neuron_print(output, layer->neurons[i]);
}

void brain_print(FILE *output, brain_t *b)
{
	assert(output && b);
	fprintf(output, "brain:\n");
	for(size_t i = 0; i < b->depth; i++)
		layer_print(output, i, b->layers[i]);
}

brain_t *brain_new(bool rand, size_t length, size_t depth)
{
	assert(length && depth);
	brain_t *b   = allocate(sizeof(*b));
	b->length    = length;
	b->depth     = depth < 2 ? 2 : depth;
	b->inputs    = allocate(sizeof(b->inputs[0]) * length);
	b->layers    = allocate(sizeof(b->layers[0]) * length);
	for(size_t i = 0; i < b->depth; i++)
		b->layers[i] = layer_new(rand, length);
	if(verbose(DEBUG) && rand)
		brain_print(stdout, b);
	return b;
}

brain_t *brain_copy(const brain_t *b)
{
	assert(b);
	brain_t *n = brain_new(false, b->length, b->depth);
	for(size_t i = 0; i < b->depth; i++)
		n->layers[i] = layer_copy(b->layers[i]);
	return n;
}

void brain_delete(brain_t *b)
{
	if(!b)
		return;
	free(b->inputs);
	for(size_t i = 0; i < b->depth; i++)
		layer_delete(b->layers[i]);
	free(b);
}

static double calculate_response(neuron_t *n, const double in[], size_t length) 
{    /* see http://www.cs.bham.ac.uk/~jxb/NN/nn.html*/
	assert(n && in && length);
	double total = n->bias;
	for(size_t i = 0; i < length; i++)
		total += in[i] * n->weights[i];
	return 1.0 / (1.0 + exp(-total));
}

void update_layer(layer_t *l, const double inputs[], size_t in_length)
{
	assert(l && inputs && in_length);
	size_t length = MIN(l->length, in_length);
	for(size_t i = 0; i < length; i++)
		l->outputs[i] = calculate_response(l->neurons[i], inputs, length);
}

void brain_update(brain_t *b, const double inputs[], size_t in_length, double outputs[], size_t out_length)
{
	assert(b->length > in_length && b->length > out_length);
	for(size_t i = 0; i < in_length; i++)
		b->inputs[i] = inputs[i];
	update_layer(b->layers[0], b->inputs, in_length);
	for(size_t i = 1; i < b->depth; i++)
		update_layer(b->layers[i], b->layers[i-1]->outputs, b->layers[i-1]->length);
	for(size_t i = 0; i < out_length; i++)
		outputs[i] = b->layers[b->depth - 1]->outputs[i];
}

static void layer_mutate(layer_t *l, unsigned *count)
{
	assert(l && count);
	for(size_t i = 0; i < l->length; i++)
		neuron_mutate(l->neurons[i], count);
}

unsigned brain_mutate(brain_t *b)
{
	assert(b);
	unsigned mutations = 0;
	for(size_t i = 1; i < b->depth; i++)
		layer_mutate(b->layers[i], &mutations);
	return mutations;
}

