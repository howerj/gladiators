/** @file       brain.c
 *  @brief      A multilayer neural network
 *  @author     Richard Howe (2016)
 *  @license    MIT <https://opensource.org/licenses/MIT>
 *  @email      howe.r.j.89@gmail.com */
#include "brain.h"
#include "util.h"
#include "vars.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
	size_t weight_count;
	double bias;
	double alpha; /**< arbitrary parameter for neurons function */
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
		r = -r;
	return r + original;
}

static neuron_t *neuron_new(bool rand, size_t length)
{
	neuron_t *n = allocate(sizeof(*n) + sizeof(n->weights[0])*length);
	n->bias         = rand ? randomer(n->bias)  : 1.0;
	n->alpha        = rand ? randomer(n->alpha) : 1.0;
	n->weight_count = length;
	for(size_t i = 0; i < length; i++)
		n->weights[i] = rand ? randomer(n->weights[i]) : 1.0; 
	return n;
}

static void neuron_delete(neuron_t *n)
{
	free(n);
}

static layer_t *layer_new(bool alloc_neurons, bool rand, size_t length)
{
	layer_t *l = allocate(sizeof(*l));
	l->length  = length;
	l->outputs = allocate(sizeof(l->outputs[0]) * length);
	l->neurons = allocate(sizeof(l->neurons[0]) * length); 
	for(size_t i = 0; alloc_neurons && i < length; i++)
		l->neurons[i] = neuron_new(rand, length);
	return l;
}

static void layer_delete(layer_t *l)
{
	if(!l)
		return;
	if(l->neurons)
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
	dst->alpha = src->alpha;
	for(size_t i = 0; i < src->weight_count; i++)
		dst->weights[i] = src->weights[i];
}

static layer_t *layer_copy(layer_t *l)
{
	layer_t *n = layer_new(true, false, l->length);
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

static void neuron_mutate(neuron_t *n, size_t depth, unsigned *count)
{
	assert(n && count);
	n->bias = mutation(n->bias, n->weight_count*depth, count);
	n->alpha = mutation(n->bias, n->weight_count*depth, count);
	for(size_t i = 0; i < n->weight_count; i++)
		n->weights[i] = mutation(n->weights[i], n->weight_count*depth, count);
}

/**@note these will eventually be removed, when the converse of the scanner
 * function is made */
static cell_t c_weights = { .type = SYMBOL, .p.string = "weights" };
static cell_t c_layer   = { .type = SYMBOL, .p.string = "layer" };
static cell_t c_layers  = { .type = SYMBOL, .p.string = "layers" };

static cell_t *neuron_serialize(neuron_t *n)
{
	cell_t *head = cons(&c_weights, nil());
	cell_t *r = printer("neuron (bias %f) (alpha %f) %x", n->bias, n->alpha, head);
	assert(r);
	cell_t *op = head;
	for(size_t i = 0; i < n->weight_count; op = cdr(op), i++)
		setcdr(op, cons(mkfloat(n->weights[i]), nil()));
	return r;
}

static cell_t *layer_serialize(layer_t *layer)
{
	assert(layer);
	cell_t *head = cons(&c_layer, nil());
	cell_t *op   = head;
	for(size_t i = 0; i < layer->length; op = cdr(op), i++)
		setcdr(op, cons(neuron_serialize(layer->neurons[i]), nil()));
	return head;
}

cell_t *brain_serialize(brain_t *b)
{
	cell_t *head = cons(&c_layers, nil());
	cell_t *r    = printer("brain (depth %d) (length %d) %x", (intptr_t)(b->depth), (intptr_t)(b->length), head);
	assert(r);
	cell_t *op = head;
	for(size_t i = 0; i < b->depth; op = cdr(op), i++)
		setcdr(op, cons(layer_serialize(b->layers[i]), nil()));
	return r;
}

brain_t *brain_new(bool alloc_layers, bool rand, size_t length, size_t depth)
{
	brain_t *b   = allocate(sizeof(*b));
	b->length    = length < 1 ? 1 : length;
	b->depth     = depth  < 2 ? 2 : depth;
	b->inputs    = allocate(sizeof(b->inputs[0]) * length);
	b->layers    = allocate(sizeof(b->layers[0]) * depth);
	for(size_t i = 0; alloc_layers && i < b->depth; i++)
		b->layers[i] = layer_new(true, rand, length);
	return b;
}

brain_t *brain_copy(const brain_t *b)
{
	assert(b);
	brain_t *n = brain_new(false, false, b->length, b->depth);
	for(size_t i = 0; i < b->depth; i++)
		n->layers[i] = layer_copy(b->layers[i]);
	return n;
}

void brain_delete(brain_t *b)
{
	if(!b)
		return;
	free(b->inputs);
	if(b->layers)
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
	switch(brain_activation_function) {
	case LOGISTIC_FUNCTION_E: 
				if(total < -45) return 0; /*overflow on exp*/
				if(total >  45) return 1; /*underflow on exp*/
				return 1.0 / (1.0 + exp(-total)); 
	case TANH_FUNCTION_E:     
				return tanh(total);
	case ATAN_FUNCTION_E:
				return atan(total);
	case IDENTITY_FUNCTION_E: 
				return total;
	case BINARY_STEP_FUNCTION_E:
				return total >= 0;
	case RECTIFIER_FUNCTION_E:
				return MAX(0, total);
	case SIN_FUNCTION_E:
				return sin(total);
	default:                
				break;
	}
	error("invalid calculation method: %u", brain_activation_function);
	return 0.0;
}

void update_layer(layer_t *l, const double inputs[], size_t in_length, size_t start)
{
	assert(l && inputs && in_length);
	size_t length = MIN(l->length, in_length);
	for(size_t i = start; i < length; i++)
		l->outputs[i] = calculate_response(l->neurons[i], inputs, length);
}

void brain_update(brain_t *b, const double inputs[], size_t in_length, double outputs[], size_t out_length)
{
	for(size_t i = 0; i < in_length; i++)
		b->inputs[i] = inputs[i];
	update_layer(b->layers[0], b->inputs, in_length, 0);
	if(brain_mix_in_feedback && ((b->length - in_length) < b->length))
		update_layer(b->layers[0], b->layers[b->depth - 1]->outputs, b->length - in_length, in_length);
	for(size_t i = 1; i < b->depth; i++)
		update_layer(b->layers[i], b->layers[i-1]->outputs, b->layers[i-1]->length, 0);
	for(size_t i = 0; i < out_length; i++)
		outputs[i] = b->layers[b->depth - 1]->outputs[i];
}

static void layer_mutate(layer_t *l, size_t depth, unsigned *count)
{
	assert(l && count);
	for(size_t i = 0; i < l->length; i++)
		neuron_mutate(l->neurons[i], depth, count);
}

unsigned brain_mutate(brain_t *b)
{
	assert(b);
	unsigned mutations = 0;
	for(size_t i = 0; i < b->depth; i++)
		layer_mutate(b->layers[i], b->depth, &mutations);
	return mutations;
}

neuron_t *neuron_deserialize(cell_t *c, size_t length)
{
	double bias = 0, alpha = 0;
	cell_t *weights;
	int r = scanner(c, "neuron (bias %f) (alpha %f) (weights %l)", &bias, &alpha, &weights);
	if(r < 0 || !weights) {
		warning("neuron deserialization failed: %d", r);
		return NULL;
	}
	neuron_t *n = neuron_new(false, length);
	for(size_t i = 0 ; type(weights) != NIL && i < length; i++, weights = cdr(weights)) {
		cell_type_e wt = type(car(weights));
		if(wt != FLOATING) {
			warning("incorrect weight type %u", wt);
			goto fail;
		}
		n->weights[i] = FLT(car(weights));
	}
	return n;
fail:
	neuron_delete(n);
	return NULL;
}

layer_t *layer_deserialize(cell_t *c, size_t length)
{
	layer_t *l = layer_new(false, false, length);
	c = cdr(c);
	for(size_t i = 0; type(c) != NIL && i < length; i++, c = cdr(c)) {
		l->neurons[i] = neuron_deserialize(car(c), length);
		if(!(l->neurons[i])) {
			warning("layer deserialization failed");
			goto fail;
		}
	}
	return l;
fail:
	layer_delete(l);
	return NULL;
}

brain_t *brain_deserialize(cell_t *c)
{
	unsigned depth = 0, length = 0;
	cell_t *layers = NULL;
	int r = scanner(c, "brain (depth %u) (length %u) (layers %l ) ", &depth, &length, &layers);
	if(r < 0 || layers == NULL)
		return NULL;
	brain_t *b = brain_new(false, false, length, depth);
	unsigned i;
	for(i = 0; type(layers) != NIL; i++, layers = cdr(layers)) {
		if(type(car(layers)) != CONS) {
			warning("invalid configuration: layer is not list");
			return NULL;
		}
		b->layers[i] = layer_deserialize(car(layers), length);
		if(!b->layers[i]) {
			warning("layers deserialization failed");
			goto fail;
		}
	}
	return b;
fail:
	brain_delete(b);
	return NULL;
}

/**@note different crossover strategies should be experimented with, either
 * crossing over at random points, or crossing at finer points within layers */
brain_t *brain_crossover(brain_t *a, brain_t *b)
{
	assert(a && b);
	assert(a->depth == b->depth && a->length == b->length);
	brain_t *c = allocate(sizeof(*c));
	c->depth = a->depth;
	c->length = a->length;
	c->inputs = allocate(sizeof(c->inputs[0]) * c->length);
	c->layers = allocate(sizeof(c->layers[0]) * c->depth);
	for(size_t i = 0; i < c->depth; i++) {
		if(i & 1)
			c->layers[i] = layer_copy(a->layers[i]);
		else
			c->layers[i] = layer_copy(b->layers[i]);
	}
	return c;
}

