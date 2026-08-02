#include "header/neurons.h"
#include "header/image.h"
#include "header/math.h"

double correctCounter = 0;
double counter = 0;

struct Layer* CreateLayer(size_t currentLayerNeuronCount, size_t nextLayerNeuronCount, struct Mat* weights, struct Mat* biases){
    if (currentLayerNeuronCount <= 0 || nextLayerNeuronCount <= 0){
        printf("Error: CreateLayer, neuronCounts must be > 0\n");
        return NULL;
    }
    if (weights == NULL)
        weights = MatCreate(nextLayerNeuronCount, currentLayerNeuronCount, NULL, InitWeights);
    if (biases == NULL)
        biases = MatCreate(nextLayerNeuronCount, 1, NULL, InitBiases);
    struct Mat* preactivation = MatCreate(currentLayerNeuronCount, 1, NULL, NULL);
    struct Mat* activation = MatCreate(currentLayerNeuronCount, 1, NULL, NULL);
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

struct Mat* ComputePreActivation(struct Layer* layer, struct Layer* nextLayer){
    if (layer == NULL || nextLayer == NULL){
        printf("Error: ComputePreActivation, layer or nextLayer are NULL\n");
        return NULL;
    }

    struct Mat* preActivation = MatMult(layer->weights, layer->activation);

    if (preActivation == NULL){
        printf("Error: ComputePreActivation, MatMult returned NULL\n");
        return NULL;
    }
    int errorCode = MatAddInternal(preActivation, layer->biases);

    if (errorCode != 0){
        printf("Error: ComputePreActivation, MatAddInternal returned %d\n", errorCode);
        MatDestroy(preActivation);
        return NULL;
    }
    return preActivation;
}

int ComputeActivation(struct Layer* layer, struct Layer* nextLayer){ // calculate the activation of the current layer using the activation of the previous one
    struct Mat* preActivation = ComputePreActivation(layer, nextLayer);
    if (preActivation == NULL){
        printf("Error: ComputeActivation, ComputePreActivation returned NULL\n");
        return 1;
    }

    struct Mat* activation = NULL;

    if (nextLayer->biases->row == nextLayer->biases->col == 1){ // nextlayer is the last layer
        activation = MatCreate(preActivation->row, 1, NULL, NULL);
        Softmax(preActivation, activation);
    }
    else
        activation = MatFunc(preActivation, Relu);

    MatDestroy(nextLayer->preActivation);
    MatDestroy(nextLayer->activation);
    nextLayer->preActivation = preActivation;
    nextLayer->activation = activation;
    return 0;
}

int ForwardPass(struct Network* network){
    int i = 0;
    while(i < network->layerCount - 1){
        int errorCode = ComputeActivation(network->layers[i], network->layers[i+1]);
        if (errorCode != 0){
            printf("Error: ForwardPass, ComputeActivation between layer %d and %d failed and returned %d\n", i, i+1, errorCode);
            return 1;
        }
        i++;
    }
    return 0;
}

int Train(struct Network* network, char** sample, size_t sampleSize, struct Mat* answer[]){
    if (sampleSize < 1||network == NULL || sample == NULL || answer == NULL){
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

    MatDestroy(network->layers[0]->activation); // destroy the first activation
    network->layers[0]->activation = NULL;

    for(size_t k = 0; k < sampleSize; k++){
        struct Mat* grayScale = GetGrayScaleMatrix(sample[k]);
        if (grayScale == NULL){
            printf("Error: Train, the grayScale matrix is NULL\n");
            errorCode = 3;
            goto clear;
        }
        // use the grayScale as the activation value of the input layer
        network->layers[0]->activation = grayScale;

        errorCode = ForwardPass(network);
        if (errorCode != 0){
            printf("Error: Train, ForwardPass returned %d\n", errorCode);
            goto clear;
        }

        int i = network->layerCount - 1;

        struct Mat* lastLayerActivation = network->layers[i]->activation;
        double biggest = lastLayerActivation->data[0][0];
        int biggestIndex = 0;
        for(int j = 1; j < lastLayerActivation->row; j++){
            double n = lastLayerActivation->data[j][0];
            if (n > biggest){
                biggest = n;
                biggestIndex = j;
            }
        }

        counter++;
        if (biggestIndex == k)
            correctCounter++;
        printf("SCORE: %f\n", correctCounter/counter);

        // BACKPROBAGATION----------------
        // error  of the last layer
        // this block perform the calculation (A^n - Y)
        // From what I calculated it should be (A^n - Y) ⊙ f'(Z^n) but we use softmax for the last layer so some magic happens and we remove the last term
        struct Mat* delta = MatSub(network->layers[i]->activation, answer[k]);

        while(i > 0){
            struct Layer* previousLayer = network->layers[i-1];

            // gradiant of the weights: weights = weights + delta*transpose(previousLayer.activation)
            struct Mat* transpose = MatTranspose(previousLayer->activation);
            struct Mat* gradiant = MatMult(delta, transpose);
            MatDestroy(transpose);
            if (weightGradiants[i-1] == NULL){
                weightGradiants[i-1] = gradiant;
            }
            else{
                MatAddInternal(weightGradiants[i-1], gradiant);
                MatDestroy(gradiant);
            }

            // gradiant of the biases : biases = biases + delta
            if (biasesGradiants[i-1] == NULL)
                biasesGradiants[i-1] = MatCopy(delta);
            else
                MatAddInternal(biasesGradiants[i-1], delta);

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
        MatDestroy(delta); // destroy the last delta

        MatDestroy(grayScale);
        network->layers[0]->activation = NULL;
    }

    // apply the gradiant to all layers at the end of the training
    for(size_t i = 0; i < (network->layerCount)-1; i++){
        struct Layer* currentLayer = network->layers[i];
        currentLayer->biases = MatSubInternal(currentLayer->biases, MatScalarInternal(biasesGradiants[i], network->learningRate));
        currentLayer->weights = MatSubInternal(currentLayer->weights, MatScalarInternal(weightGradiants[i], network->learningRate));
    }

    clear:
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
        answer10[i] = MatCreate(10, 1, NULL, NULL);
        for(int j = 0; j < 10; j++){
            answer10[i]->data[j][0] = i==j ? 1 : 0;
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

    int cond = weights != NULL && biases != NULL;
    for(size_t i = 0; i < layerCount-1; i++){
        struct Mat* weight = cond ? weights[i] : NULL;
        struct Mat* bias = cond ? biases[i] : NULL;
        network->layers[i] = CreateLayer(neuronsPerLayer[i], neuronsPerLayer[i+1], weight, bias);
        if (network->layers[i] == NULL){
            printf("Error: CreateNetwork, CreateLayer inside the loop returned NULL\n");
            return NULL;
        }
    }
    network->layers[layerCount-1] = CreateLayer(neuronsPerLayer[layerCount-1], 1, NULL, NULL); // final layer (no weights nor biases) we can put any number as second argument
    if (network->layers[layerCount-1] == NULL){
        printf("Error: CreateNetwork, CreateLayer outside the loop returned NULL\n");
        return NULL;
    }
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
