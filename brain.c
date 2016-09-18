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

struct brain_t {
	size_t length;
	double   *input;
	neuron_t **layer1;
	double   *layer1out;
	neuron_t **layer2;
	double   *layer2out;
};

static prng_t *rstate;

static double randomer(double original)
{
	if(!rstate) /**@warning not threadsafe*/
		rstate = new_prng(13);
	double r = prngf(rstate) * brain_max_weight;
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

static void neuron_copy_over(neuron_t *dst, const neuron_t *src)
{
	dst->bias = src->bias;
	dst->weight_count = src->weight_count;
	dst->threshold = src->threshold;
	for(size_t i = 0; i < src->weight_count; i++)
		dst->weights[i] = src->weights[i];
}

static void neuron_print(FILE *output, neuron_t *n)
{
	fprintf(output, "\tlength:  %zu\n", n->weight_count);
	fprintf(output, "\tbias:    %f\n",  n->bias);
	fprintf(output, "\tweights:\n");
	for(size_t i = 0; i < n->weight_count; i++)
		fprintf(output, "\t\t%f\n", n->weights[i]);
}

static void layer_print(FILE *output, size_t length, unsigned depth, neuron_t **layer)
{
	fprintf(output, "\tlayer %d:\n", depth);
	for(size_t i = 0; i < length; i++)
		neuron_print(output, layer[i]);
}

void brain_print(FILE *output, brain_t *b)
{
	fprintf(output, "brain:\n");
	layer_print(output, b->length, 0, b->layer1);
	layer_print(output, b->length, 0, b->layer2);
}

brain_t *brain_new(bool rand, size_t length)
{
	brain_t *b   = allocate(sizeof(*b));
	b->input     = allocate(sizeof(b->input[0])    * length);
	b->layer1    = allocate(sizeof(b->layer1[0])   * length);
	b->layer1out = allocate(sizeof(b->layer1out[0])* length);
	b->layer2    = allocate(sizeof(b->layer2[0])   * length);
	b->layer2out = allocate(sizeof(b->layer2out[0])* length);
	for(size_t i = 0; i < length; i++) {
		b->layer1[i] = neuron_new(rand, length);
		b->layer2[i] = neuron_new(rand, length);
	}
	b->length = length;
	if(debug_mode && rand)
		brain_print(stdout, b);
	return b;
}

brain_t *brain_copy(const brain_t *b)
{
	brain_t *n = brain_new(false, b->length);
	for(size_t i = 0; i < b->length; i++) {
		neuron_copy_over(n->layer1[i], b->layer1[i]);
		neuron_copy_over(n->layer2[i], b->layer2[i]);
	}
	return n;
}

void brain_delete(brain_t *b)
{
	if(!b)
		return;
	free(b->input);
	free(b->layer1);
	free(b->layer1out);
	free(b->layer2);
	free(b->layer2out);
	free(b);
}

static double calculate_response(neuron_t *n, double in[], size_t len) 
{    /* see http://www.cs.bham.ac.uk/~jxb/NN/nn.html*/
	assert(n->weight_count == len);
	double total = n->bias;
	for(size_t i = 0; i < len; i++)
		total += in[i] * n->weights[i];
	return 1.0 / (1.0 + exp(-total));
}

void brain_update(brain_t *b, const double inputs[], size_t in_length, double outputs[], size_t out_length)
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

void brain_mutate(brain_t *b)
{
	for(size_t i = 0; i < b->length; i++) {
		neuron_mutate(b->layer1[i]);
		neuron_mutate(b->layer2[i]);
	}
}

