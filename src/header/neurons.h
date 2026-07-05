#pragma once
#include "math.h"

struct Layer{
    struct Mat* weights;
    struct Mat* biases;
    struct Mat* activation;
};

struct Layer* CreateLayer(size_t currentLayerNeuronCount, size_t nextLayerNeuronCount, struct Mat* weights, struct Mat* biases);
void DestroyLayer(struct Layer* layer);
int ForwardPass(struct Layer* layer, struct Mat* previousActivation);
