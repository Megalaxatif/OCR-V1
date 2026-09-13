#include "header/neurons.h"
#include "header/image.h"
#include "header/math.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include "header/init.h"
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

int ForwardPass(struct Network* network, struct Mat* input){
    input = InvertForwardPassGrayScaleMatrix(input); // needed if we are using black on white training images
    network->layers[0]->activation = input;
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

int GetGuessedDigit(struct Network* network){
    if (network == NULL){
        printf("Error: GetGuessedDigitIndex, invalid argument\n");
        return -1;
    }
    struct Mat* lastLayerActivation = network->layers[network->layerCount - 1]->activation;
    double biggest = lastLayerActivation->data[0][0];
    int biggestIndex = 0;
    for(int j = 1; j < lastLayerActivation->row; j++){
        double n = lastLayerActivation->data[j][0];
        if (n > biggest){
            biggest = n;
            biggestIndex = j;
        }
    }
    return biggestIndex; // the guessed digit corresponds to this index
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

        int guessedDigit = SolveImage(sample[k], network, 1);
        counter++;
        if (guessedDigit == k){
            correctCounter++;
            printf("hit  | ");
        }
        else printf("miss | ");
        printf("%s vs %d | SCORE: %f\n", sample[k], guessedDigit, correctCounter/counter);

        // BACKPROBAGATION----------------
        // error  of the last layer
        // this block perform the calculation (A^n - Y)
        // From what I calculated it should be (A^n - Y) ⊙ f'(Z^n) but we use softmax for the last layer so some magic happens and we remove the last term
        int i = network->layerCount - 1;
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
        // important
        MatDestroy(network->layers[0]->activation);
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

void PrintDigitGrayScales(struct Mat** digitGrayScale, size_t grayScaleCount){
    if (digitGrayScale == NULL){
        printf("Error: MatPrint, matrix pointer is NULL\n");
        return;
    }
    for(int i = 0; i < grayScaleCount; i++){
        struct Mat* currentGrayScale = digitGrayScale[i];
        if (currentGrayScale->col != 1 || currentGrayScale->row != NETWORK_IMG_SIZE*NETWORK_IMG_SIZE){
            printf("Error: PrintDigitGrayScale, the matrix given doesn't have valid dimensions for a digit grayscale");
            return;
        }
        for(size_t i = 0; i < NETWORK_IMG_SIZE; i++){
            for(size_t j = 0; j < NETWORK_IMG_SIZE; j++){
                printf("%.1f ", currentGrayScale->data[i*NETWORK_IMG_SIZE+j][0]);
            }
            printf("\n");
        }
        printf("\n");
    }
}

int* SolveGrayScales(struct Mat** grayScales, size_t grayScaleCount, struct Network* network){
    if (grayScales == NULL || grayScaleCount <= 0 || network == NULL){
        printf("Error: SolveGrayScales, invalid argument\n");
        return NULL;
    }
    int* digits = malloc(grayScaleCount * sizeof(int));
    for(int i = 0; i < grayScaleCount; i++){
        if (grayScales[i] == NULL)
            digits[i] = -1;
        else {
            int errorCode = ForwardPass(network, grayScales[i]);

            if (errorCode != 0){
                printf("Error: SolveGrayScales, ForwardPass returned %d\n", errorCode);
                free(digits);
                return NULL;
            }
            int guessedDigit = GetGuessedDigit(network);
            digits[i] = guessedDigit;
        }
    }
    return digits;
}

int SolveImage(char* path, struct Network* network, int isTraining){
    SDL_Surface* trainingSurface = IMG_Load(path);
    if (trainingSurface == NULL){
        printf("Error: SolveImage, impossible to load the image at %s\n", path);
        return -1;
    }
    if (trainingSurface->w != NETWORK_IMG_SIZE || trainingSurface->h != NETWORK_IMG_SIZE){
        printf("Error: SolveImage, invalid image dimentions\n");
        return -2;
    }
    SDL_Surface *tmp = SDL_ConvertSurfaceFormat(trainingSurface, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(trainingSurface);
    trainingSurface = tmp;

    struct Mat* grayScale = GetForwardPassGrayScaleMatrix(trainingSurface);
    SDL_FreeSurface(trainingSurface);

    if (grayScale == NULL){
        printf("Error: SolveImage, GetForwardPassGrayScaleMatrix returned NULL\n");
        return -2;
    }

    int errorCode = ForwardPass(network, grayScale);

    // the nework must not delete the first activation matrix if it's training because of backpropagation
    if (!isTraining){
        MatDestroy(grayScale);
        network->layers[0]->activation = NULL;
    }

    if (errorCode != 0){
        printf("Error: SolveImage, ForwardPass returned %d\n", errorCode);
        return -3;
    }

    int guessedDigit = GetGuessedDigit(network);

    return guessedDigit;
}

int* SolveSudoku(char* sudokuPath, struct Network* network){
    if (sudokuPath == NULL || network == NULL){
        printf("Error: SolveSudoku, invalid argument\n");
        return NULL;
    }
    struct Mat* gridGrayScale = GetGridGrayScaleMatrix(sudokuPath);
    if (gridGrayScale == NULL){
        printf("Error: SolveSudoku, gridGrayScale is NULL\n");
        return NULL;
    }
    struct SDL_Rect* digitRects = NULL;
    SDL_Texture** digitTextures = NULL;
    struct Mat** digitGrayScales = NULL;
    int* digits = NULL;

    size_t horizontalLineCount = 0;
    SDL_Rect* horizontalLines = ScanHorizontalLines(gridGrayScale, &horizontalLineCount);
    size_t horizontalBlockCount = 0;
    SDL_Rect* horizontalBlocks = ConvertHorizontalLinesToBlocks(horizontalLines, horizontalLineCount, &horizontalBlockCount); // this function destroys horizontalLines

    size_t verticalLineCount = 0;
    SDL_Rect* verticalLines = ScanVerticalLines(gridGrayScale, &verticalLineCount);
    size_t verticalBlockCount = 0;
    SDL_Rect* verticalBlocks = ConvertVerticalLinesToBlocks(verticalLines, verticalLineCount, &verticalBlockCount); // this function destroys verticalLines

    int ret = SortBlocks(&horizontalBlocks, &verticalBlocks, &horizontalBlockCount, &verticalBlockCount);
    if (ret != 0){
        printf("Error: SolveSudoku, SortBlocks returned %d\n", ret);
        goto cleanup;
    }

    digitRects = GetSudokuDigitRects(horizontalBlocks, verticalBlocks); // always return a 81 array
    if (digitRects == NULL){
        printf("Error: SolveSudoku, GetSudokuDigitRects returned NULL\n");
        goto cleanup;
    }
    digitTextures = GetSudokuDigitTextures(digitRects, sudokuPath);
    if (digitTextures == NULL){
        printf("Error: SolveSudoku, GetSudokuDigitTextures returned NULL\n");
        goto cleanup;
    }
    // debug -----
    // SDL_SetRenderTarget(renderer, NULL);
    // for(int i = 0; i < 9; i++){
    //     for(int j = 0; j < 9; j++){
    //         SDL_RenderCopy(renderer, digitTextures[i*9+j], NULL, digitRects+i*9+j);
    //     }
    // }
    //-------
    digitGrayScales = ConvertTexturesToGrayScale(digitTextures, 81);
    if (digitGrayScales == NULL){
        printf("Error: SolveSudoku, ConvertTexturesToGrayScale returned NULL\n");
        goto cleanup;
    }
    digitGrayScales = DeleteBlankGrayScales(digitGrayScales, 81);
    // for(int i = 0; i < 81; i++){
    //     struct Mat* tmp = InvertForwardPassGrayScaleMatrix(digitGrayScales[i]);
    //      DrawDigitGrayScales(&tmp, digitRects + i, 1, gridGrayScale);
    // }
    //PrintDigitGrayScales(digitGrayScales, 81); // debug
    // SDL_Surface* sudoku = IMG_Load(sudokuPath);
    // SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, sudoku);
    // SDL_FreeSurface(sudoku);
    // SDL_SetRenderTarget(renderer, NULL);
    // SDL_RenderCopy(renderer, texture, NULL, NULL);
    // SDL_DestroyTexture(texture);
    DrawDigitGrayScales(digitGrayScales, digitRects, 81, gridGrayScale);
    //DrawRects(digitRects, 81, gridGrayScale, (SDL_Color){0,255,0,255});

    digits = SolveGrayScales(digitGrayScales, 81, network);
    if (digits == NULL){
        printf("Error: SolveSudoku, SolveGrayScales returned NULL\n");
        goto cleanup;
    }

    cleanup:
    // destroy grid
    MatDestroy(gridGrayScale);

    // free lines and blocks
    free(horizontalLines);
    free(horizontalBlocks);
    free(verticalLines);
    free(verticalBlocks);

    // free digitRects
    if (digitRects != NULL){
        free(digitRects);
    }

    // destroy digitTextures
    if (digitTextures != NULL){
        for (int i = 0; i < 81; i++)
            SDL_DestroyTexture(digitTextures[i]);
        free(digitTextures);
    }

    if (digitGrayScales != NULL){
        for (int i = 0; i < 81; i++)
            MatDestroy(digitGrayScales[i]);
        network->layers[0]->activation = NULL; // important
        free(digitGrayScales);
    }

    return digits;
}
