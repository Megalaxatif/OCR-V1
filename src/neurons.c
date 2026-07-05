#include "header/neurons.h"
#include "header/math.h"


struct Layer* CreateLayer(size_t currentLayerNeuronCount, size_t nextLayerNeuronCount, struct Mat* weights, struct Mat* biases){
    if (weights == NULL)
        weights = MatCreate(nextLayerNeuronCount, currentLayerNeuronCount, NULL);
    if (biases == NULL)
        biases = MatCreate(currentLayerNeuronCount, 1, NULL);
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

int ForwardPass(struct Layer* layer, struct Mat* previousActivation){ // calculate the activation of the current layer using the activation of the previous one
    struct Mat* newActivation = MatMult(layer->weights, previousActivation);
    if (newActivation == NULL) return 1;
    newActivation = MatAdd(newActivation, layer->biases);
    MatDestroy(layer->activation);
    layer->activation = newActivation;
    return 0;
}
