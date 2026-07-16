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
    struct Mat* preactivation = MatCreate(currentLayerNeuronCount, 1, NULL);
    struct Mat* activation = MatCreate(currentLayerNeuronCount, 1, NULL);
    struct Layer* newLayer = malloc(sizeof(struct Layer));

    newLayer->preActivation = preactivation;
    newLayer->activation = activation;
    newLayer->biases = biases;
    newLayer->weights = weights;

    return newLayer;
}

void DestroyLayer(struct Layer* layer){
    if (layer == NULL) return;
    if (layer->preActivation != NULL)
        MatDestroy(layer->preActivation);
    if (layer->activation != NULL)
        MatDestroy(layer->activation);
    if (layer->biases != NULL)
        MatDestroy(layer->biases);
    if (layer->weights != NULL)
        MatDestroy(layer->weights);
    free(layer);
}

int ForwardPass(struct Layer* layer, struct Layer* nextLayer){ // calculate the activation of the current layer using the activation of the previous one
    if  (layer == NULL || nextLayer == NULL){
        printf("Error: ForwardPass, layer or nextLayer are NULL");
        return 1;
    }
    struct Mat* preActivation = MatMult(layer->weights, layer->activation);
    if (preActivation == NULL){
        printf("Error: ForwardPass, preActivation is NULL\n");
        return 2;
    }
    preActivation = MatAdd(preActivation, layer->biases);
    if (preActivation == NULL){
        printf("Error: ForwardPass, preActivation is NULL\n");
        return 3;
    }
    struct Mat* activation = MatFunc(preActivation, Relu);
    if (activation == NULL){
        printf("Error: ForwardPass, activation is NULL\n");
        return 4;
    }
    MatDestroy(nextLayer->preActivation);
    MatDestroy(nextLayer->activation);
    nextLayer->preActivation = preActivation;
    nextLayer->activation = activation;
    return 0;
}

int Train(struct Network* network, char* imgFileName, struct Mat* answer){
    if (network == NULL || imgFileName == NULL || answer == NULL){
        printf("Error: Train, network or imgFileName or the answer matrix are NULL\n"); // split the errors
        return 1;
    }
    if (network->layerCount < 3){
        printf("Error: Train, invalid layerCount, you must use at least 3 layers\n");
        return 2;
    }
    struct Mat* grayScale = GetGrayScaleMatrix(imgFileName);
    if (grayScale == NULL){
        printf("Error: Train, the grayScale matrix is NULL\n");
        return 3;
    }
    // use the grayScale as the activation value of the input layer
    MatDestroy(network->layers[0].activation);
    network->layers[0].activation = grayScale;

    int i = 0;
    while(i < network->layerCount){
        ForwardPass(network->layers + i, network->layers + i + 1);
        i++;
    }
    MatPrint(network->layers[i].activation); // print the output layer

    // error  of the last layer n
    // this line perform the calculation : (A^n - Y) ⊙ f'(Z^n)
    struct Mat* sub = MatSub(network->layers[i].activation, answer);
    struct Mat* func = MatFunc(network->layers[i].preActivation, ReluPrime);
    struct Mat* delta = MatHadamard(sub,func);
    MatDestroy(sub);
    MatDestroy(func);

    struct Mat* tmp = NULL;

    while(i > 0){
        struct Layer currentLayer = network->layers[i];
        struct Layer previousLayer = network->layers[i-1];

        // gradiant of the biases : biases = biases + learningRate*delta
        struct Mat* scalar = MatScalar(delta, network->learningRate);
        tmp = currentLayer.biases;
        currentLayer.biases = MatAdd(tmp, scalar);
        MatDestroy(tmp);

        // gradiant of the weights: weights = weights + (learningRate*delta)*transpose(previousLayer.activation)
        struct Mat* transpose = MatTranspose(previousLayer.activation);
        struct Mat* gradiant = MatMult(scalar, transpose);
        MatDestroy(transpose);
        tmp = currentLayer.weights;
        currentLayer.weights = MatAdd(tmp, gradiant);
        MatDestroy(tmp);
        MatDestroy(scalar);
        MatDestroy(gradiant);

        // calculate the new delta
        if (i > 1){ // the delta is undefined for the input layer
            struct Mat* transpose = MatTranspose(previousLayer.weights);
            struct Mat* mat1 = MatMult(transpose, delta);
            MatDestroy(transpose);

            struct Mat* func = MatFunc(previousLayer.preActivation, ReluPrime);

            MatDestroy(delta);
            delta = MatHadamard(mat1, func);
            MatDestroy(mat1);
            MatDestroy(func);
        }
        i--;
    }
    return 0;
}
