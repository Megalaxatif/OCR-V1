#include "header/neurons.h"
#include "header/image.h"
#include "header/math.h"
#include "header/init.h"

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


int Train(struct Network* network, char* imgFileNames[], size_t fileCount, struct Mat* answer[]){
    if (fileCount < 1||network == NULL || imgFileNames == NULL || *imgFileNames == NULL || answer == NULL || *answer == NULL){
        printf("Error: Train, invalid arguments\n");
        return 1;
    }
    if (network->layerCount < 3){
        printf("Error: Train, invalid layerCount in the network structure. You must use at least 3 layers\n");
        return 2;
    }

    int errorCode = 0;

    struct Mat** weightGradiants = calloc(network->layerCount-1, sizeof(struct Mat*));   // weights gradiant list for each layer
    struct Mat** biasesGradiants = calloc(network->layerCount-1, sizeof(struct Mat*));   // biases gradiant list for each layer

    for(size_t k = 0; k < fileCount; k++){
        struct Mat* grayScale = GetGrayScaleMatrix(imgFileNames[k]);
        if (grayScale == NULL){
            printf("Error: Train, the grayScale matrix is NULL\n");
            errorCode = 3;
            break;
        }
        // use the grayScale as the activation value of the input layer
        MatDestroy(network->layers[0]->activation);
        network->layers[0]->activation = grayScale;

        int i = 0;
        while(i < network->layerCount){
            ForwardPass(network->layers[i], network->layers[i+1]);
            i++;
        }
        MatPrint(network->layers[i]->activation); // print the output layer

        // BACKPROBAGATION----------------
        // error  of the last layer n
        // this bloc perform the calculation : (A^n - Y) ⊙ f'(Z^n)
        struct Mat* sub = MatSub(network->layers[i]->activation, answer[k]);
        struct Mat* func = MatFunc(network->layers[i]->preActivation, ReluPrime);
        struct Mat* delta = MatHadamard(sub,func);
        MatDestroy(sub);
        MatDestroy(func);


        while(i > 0){
            struct Layer* previousLayer = network->layers[i-1];

            // gradiant of the biases : biases = biases + learningRate*delta // TODO: move this comment where the gradiant is truly used
            if (biasesGradiants[i-1] == NULL)
                biasesGradiants[i-1] = delta;
            else
                MatAddInternal(biasesGradiants[i-1], delta);

            // gradiant of the weights: weights = weights + (learningRate*delta)*transpose(previousLayer.activation) // TODO: move this comment where the gradiant is truly used
            struct Mat* transpose = MatTranspose(previousLayer->activation);
            struct Mat* gradiant = MatMult(delta, transpose);

            if (weightGradiants[i-1] == NULL){
                weightGradiants[i-1] = gradiant;
            }
            else{
                MatAddInternal(weightGradiants[i-1], gradiant);
                MatDestroy(gradiant);
            }
            MatDestroy(transpose);

            // calculate the new delta
            if (i > 1){ // the delta is undefined for the input layer
                struct Mat* transpose = MatTranspose(previousLayer->weights);
                struct Mat* mat1 = MatMult(transpose, delta);
                MatDestroy(transpose);

                struct Mat* func = MatFunc(previousLayer->preActivation, ReluPrime);

                MatDestroy(delta);
                delta = MatHadamard(mat1, func);

                MatDestroy(mat1);
                MatDestroy(func);
            }
            i--;
        }
        MatDestroy(grayScale);
    }

    // apply the gradiant to all layers at the end of the training
    struct Mat* tmp = NULL;
    for(size_t i = 0; i < network->layerCount-1; i++){
        struct Layer* currentLayer = network->layers[i];
        tmp = currentLayer->biases;
        currentLayer->biases = MatSub(tmp, MatScalarInternal(biasesGradiants[i], network->learningRate));
        MatDestroy(tmp);

        tmp = currentLayer->weights;
        currentLayer->weights = MatSub(tmp, MatScalarInternal(weightGradiants[i], network->learningRate));
        MatDestroy(tmp);
    }

    // clear
    for(size_t i = 0; i < network->layerCount-1; i++){
        MatDestroy(weightGradiants[i]);
        MatDestroy(biasesGradiants[i]);
    }
    free(weightGradiants);
    free(biasesGradiants);
    return errorCode;
}

struct Mat** GetAnswer10(){
    struct Mat** answer10 = malloc(10*sizeof(struct Mat*));
    for(int i = 0; i < 10; i++){
        answer10[i] = MatCreate(10, 1, NULL);
        for(int j = 0; j < 10; j++){
            answer10[i]->data[j][0] = 1 ? i==j : 0;
        }
    }
    return answer10;
}

struct Network* CreateNetwork(double learningRate, size_t layerCount, int* neuronsPerLayer, struct Mat* weights[], struct Mat* biases[]){
    if (learningRate <= 0){
        printf("Error : CreateNetork, you need to have a learning Rate > 0\n");
        return NULL;
    }
    if (layerCount < 3){
        printf("Error : CreateNetwork, you need to have at least 3 layers in the network\n");
        return NULL;
    }
    if (neuronsPerLayer == NULL){
        printf("Error: CreateNetwork, neuronsPerLayer is NULL\n");
        return NULL;
    }

    struct Network* network = malloc(sizeof(struct Network));
    network->layerCount = layerCount;
    network->learningRate = learningRate;
    network->layers = malloc(layerCount*sizeof(struct Layer*));

    if (weights != NULL && biases != NULL){
        for(size_t i = 0; i < layerCount-1; i++){
            network->layers[i] = CreateLayer(neuronsPerLayer[i], neuronsPerLayer[i+1], weights[i], biases[i]);
        }
    }

    else{
        for(size_t i = 0; i < layerCount-1; i++){
            network->layers[i] = CreateLayer(neuronsPerLayer[i], neuronsPerLayer[i+1], NULL, NULL);
        }
    }
    network->layers[layerCount-1] = CreateLayer(neuronsPerLayer[layerCount-1], 1, NULL, NULL); // final layer (no weights nor biases) we can put any number as second argument
    return network;
}

void DestroyNetwork(struct Network* network){
    if (network == NULL) return;
    for(int i = 0; i < network->layerCount; i++){
        DestroyLayer(network->layers[i]);
    }
    free(network->layers);
    free(network);
}
