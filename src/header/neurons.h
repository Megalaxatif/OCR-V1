#pragma once
#include "math.h"

struct Layer{
    struct Mat* weights;       // W
    struct Mat* biases;        // B
    struct Mat* preActivation; // Z
    struct Mat* activation;    // A
};

struct Network{
    struct Layer* layers;
    size_t layerCount;
    double learningRate;
};
struct Layer* CreateLayer(size_t currentLayerNeuronCount, size_t nextLayerNeuronCount, struct Mat* weights, struct Mat* biases);
void DestroyLayer(struct Layer* layer);
int ForwardPass(struct Layer* layer, struct Layer* nextLayer);
(struct Network* network, char** imgFileNames, size_t fileCount, struct Mat** answer);
