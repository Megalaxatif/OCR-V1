#pragma once
#include "math.h"

struct Layer{
    struct Mat* weights;       // W
    struct Mat* biases;        // B
    struct Mat* preActivation; // Z
    struct Mat* activation;    // A
};

struct Network{
    struct Layer** layers;
    size_t layerCount;
    double learningRate;
};



struct Layer* CreateLayer(size_t currentLayerNeuronCount, size_t nextLayerNeuronCount, struct Mat* weights, struct Mat* biases); // explicit enough
void DestroyLayer(struct Layer* layer); // you can't be more explicit than that
int Train(struct Network* network, char** sample, size_t sampleSize, struct Mat* answer[]);
struct Mat** GetAnswer10(); // get the list of answer matrix for a training of 1 image on each digit from 0 to 9
struct Network* CreateNetwork(double learningRate, size_t layerCount, int* neuronsPerLayer, struct Mat* weights[], struct Mat* biases[]); // create a network with the weights and biases given if not NULL, otherwise create a new network
void DestroyNetwork(struct Network* network);
