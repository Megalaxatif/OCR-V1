#include "header/neurons.h"
#include "header/image.h"
#include "header/math.h"


struct Layer* CreateLayer(size_t currentLayerNeuronCount, size_t nextLayerNeuronCount, struct Mat* weights, struct Mat* biases){
    if (currentLayerNeuronCount <= 0 || nextLayerNeuronCount <= 0){
        printf("Error: CreateLayer, neuronCounts must be > 0\n");
        return NULL;
    }
    if (weights == NULL)
        weights = MatCreate(nextLayerNeuronCount, currentLayerNeuronCount, NULL);
    if (biases == NULL)
        biases = MatCreate(nextLayerNeuronCount, 1, NULL);
    struct Mat* activation = MatCreate(currentLayerNeuronCount, 1, NULL);
    struct Layer* newLayer = malloc(sizeof(struct Layer));

    newLayer->activation = activation;
    newLayer->biases = biases;
    newLayer->weights = weights;

    return newLayer;
}

void DestroyLayer(struct Layer* layer){
    if (layer == NULL) return;
    if (layer->activation != NULL)
        MatDestroy(layer->activation);
    if (layer->biases != NULL)
        MatDestroy(layer->biases);
    if (layer->weights != NULL)
        MatDestroy(layer->weights);
    free(layer);
}

int ForwardPass(struct Layer* layer, struct Layer* nextLayer){ // calculate the activation of the current layer using the activation of the previous one
    struct Mat* newActivation = MatMult(layer->weights, layer->activation);
    if (newActivation == NULL){
        printf("Error: ForwardPass, newActivation is NULL\n");
        return 1;
    }
    newActivation = MatAdd(newActivation, layer->biases);
    if (newActivation == NULL){
        printf("Error: ForwardPass, newActivation is NULL\n");
        return 2;
    }
    newActivation = MatFunc(newActivation, Relu);
    if (newActivation == NULL){
        printf("Error: ForwardPass, newActivation is NULL\n");
        return 3;
    }
    MatDestroy(nextLayer->activation);
    nextLayer->activation = newActivation;
    return 0;
}

int Train(struct Layer* network, char* imgFileName){
    if (network == NULL){
        printf("Error: Train, the network is NULL\n");
        return 1;
    }
    struct Mat* grayScale = GetGrayScaleMatrix(imgFileName);
    if (grayScale == NULL){
        printf("Error: Train, the grayScale matrix is NULL\n");
        return 2;
    }
    // use the grayScale as the activation value of the input layer
    MatDestroy(network[0].activation);
    network[0].activation = grayScale;

    int i = 0;
    while(network + i+1 != NULL){
        ForwardPass(network + i, network + i+1);
        i++;
    }
    MatPrint(network[i].activation);

    // TODO: backpropagation
}
