#include "header/files.h"
#include "header/image.h"

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
    if (layer == NULL)
        return;
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

struct Network* CreateNetwork(size_t layerCount, int* neuronsPerLayer,  struct Mat* weights[], struct Mat* biases[]){
    // NOTE if weights and biases are defined, they both are layerCount-1 elements long because the weights
    // and biases of the last layer are not used in any computation so we don't need to store them
    int cond = weights != NULL && biases != NULL;
    if (layerCount < DEFAULT_LAYER_COUNT){
        printf("Error : CreateNetwork, you need to have at least %d layers in the network\n", DEFAULT_LAYER_COUNT);
        return NULL;
    }
    if (cond && neuronsPerLayer != NULL){
        printf("WARNING: CreateNetwork, no need to use the argument neuronsPerLayer for networks initialized with already existing weights and biases\n");
        return NULL;
    }
    if (!cond && neuronsPerLayer == NULL){
        printf("Error: CreateNetwork, neuronsPerLayer is NULL\n");
        return NULL;
    }

    struct Network* network = malloc(sizeof(struct Network));
    network->layers = calloc(layerCount,sizeof(struct Layer*));
    network->layerCount = layerCount;

    for(size_t i = 0; i < layerCount-1; i++){
        struct Mat* weight = cond ? weights[i] : NULL;
        struct Mat* bias = cond ? biases[i] : NULL;
        size_t nextLayerNeuronCount = cond ? weights[i]->row : (size_t)neuronsPerLayer[i+1];
        size_t currentLayerNeuronCount = cond ? weights[i]->col : (size_t)neuronsPerLayer[i];

        struct Layer* layer = CreateLayer(currentLayerNeuronCount, nextLayerNeuronCount, weight, bias);
        if (layer == NULL){
            printf("Error: CreateNetwork, CreateLayer inside the loop returned NULL\n");
            DestroyNetwork(network);
            return NULL;
        }
        network->layers[i] = layer;
    }
    size_t lastLayerNeuronCount = cond ? weights[layerCount-2]->col : (size_t)neuronsPerLayer[layerCount-1];

    struct Layer* layer = CreateLayer(lastLayerNeuronCount, 1, NULL, NULL); // final layer (no weights nor biases) we can put any number as second argument
    if (layer == NULL){
        printf("Error: CreateNetwork, CreateLayer outside the loop returned NULL\n");
        DestroyNetwork(network);
        return NULL;
    }
    network->layers[layerCount-1] = layer;
    return network;
}

void DestroyNetwork(struct Network* network){
    printf("DESTROY NETWORK\n");
    if (network == NULL) {
        printf("WARNING: DestroyNetwork, no network to destroy (network is NULL)\n");
        return;
    }
    for(size_t i = 0; i < network->layerCount; i++)
        DestroyLayer(network->layers[i]);
    free(network->layers);
    free(network);
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

    if (nextLayer->biases->row == 1 && nextLayer->biases->col == 1){ // nextlayer is the last layer
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

int GetGuessedDigit(struct Network* network){
    if (network == NULL){
        printf("Error: GetGuessedDigitIndex, invalid argument\n");
        return -1;
    }
    struct Mat* lastLayerActivation = network->layers[network->layerCount - 1]->activation;
    double biggest = lastLayerActivation->data[0][0];
    int biggestIndex = 0;
    for(size_t j = 1; j < lastLayerActivation->row; j++){
        double n = lastLayerActivation->data[j][0];
        if (n > biggest){
            biggest = n;
            biggestIndex = j;
        }
    }
    return biggestIndex; // the guessed digit corresponds to this index
}

struct Mat** GetAnswer10(){
    struct Mat** answer10 = malloc(10*sizeof(struct Mat*));
    for(int i = 0; i < 10; i++){
        answer10[i] = MatCreate(10, 1, NULL, NULL);
        if (answer10[i] == NULL){
            printf("Error: GetAnswer10, MatCreate returned NULL \n");
            for (int j = 0; j < i; j++)
                MatDestroy(answer10[j]);
            free(answer10);
            return NULL;
        }
        for(int j = 0; j < 10; j++){
            answer10[i]->data[j][0] = i==j ? 1 : 0;
        }
    }
    return answer10;
}

int Train(struct OCR* ocr, char** sample, size_t sampleSize, struct Mat* answer[]){
    if (sampleSize < 1||ocr == NULL || sample == NULL || answer == NULL){
        printf("Error: Train, invalid arguments\n");
        return 1;
    }
    struct Network* network = ocr->network;
    if (network->layerCount < 3){
        printf("Error: Train, invalid layerCount in the network structure. You must use at least 3 layers\n");
        return 2;
    }

    int errorCode = 0;

    struct Mat** weightGradiants = calloc(network->layerCount-1, sizeof(struct Mat*));   // weights gradiant list for each layer
    struct Mat** biasesGradiants = calloc(network->layerCount-1, sizeof(struct Mat*));   // biases gradiant list for each layer

    for(size_t k = 0; k < sampleSize; k++){

        int guessedDigit = SolveImage(sample[k], network);
        if (guessedDigit < 0){
            errorCode = 4;
            goto clear;
        }
        ocr->info->counter++;
        if ((size_t)guessedDigit == k){
            ocr->info->correctCounter++;
            printf("hit  | ");
        }
        else printf("miss | ");
        printf("%s vs %d | SCORE: %f\n", sample[k], guessedDigit, ocr->info->correctCounter/ocr->info->counter);

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
            if (weightGradiants[i-1] == NULL)
                weightGradiants[i-1] = gradiant;
            else{
                MatAddInternal(weightGradiants[i-1], gradiant);
                MatDestroy(gradiant);
            }

            // gradiant of the biases : biases = biases + delta
            if (biasesGradiants[i-1] == NULL)
                biasesGradiants[i-1] = MatClone(delta);
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
    }

    // apply the gradiant to all layers at the end of the training
    for(size_t i = 0; i < (network->layerCount)-1; i++){
        struct Layer* currentLayer = network->layers[i];
        currentLayer->biases = MatSubInternal(currentLayer->biases, MatScalarInternal(biasesGradiants[i], ocr->settings->learningRate));
        currentLayer->weights = MatSubInternal(currentLayer->weights, MatScalarInternal(weightGradiants[i], ocr->settings->learningRate));
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

int SolveGrayScale(struct Network* network, struct Mat* input){
    input = InvertGrayScaleMatrix(input); // needed if we are using black on white training images
    MatCopy(input, network->layers[0]->activation); // copy the input in the activation of the first layer
    size_t i = 0;
    while(i < network->layerCount - 1){
        int errorCode = ComputeActivation(network->layers[i], network->layers[i+1]);
        if (errorCode != 0){
            printf("Error: SolveGrayScale, ComputeActivation between layer %ld and %ld failed and returned %d\n", i, i+1, errorCode);
            return 1;
        }
        i++;
    }
    return 0;
}

int* SolveGrayScales(struct Mat** grayScales, size_t grayScaleCount, struct Network* network){
    if (grayScales == NULL || grayScaleCount <= 0 || network == NULL){
        printf("Error: SolveGrayScales, invalid argument\n");
        return NULL;
    }
    int* digits = malloc(grayScaleCount * sizeof(int));
    for(size_t i = 0; i < grayScaleCount; i++){
        if (grayScales[i] == NULL)
            digits[i] = -1;
        else {
            int errorCode = SolveGrayScale(network, grayScales[i]);

            if (errorCode != 0){
                printf("Error: SolveGrayScales, SolveGrayScale returned %d\n", errorCode);
                free(digits);
                return NULL;
            }
            int guessedDigit = GetGuessedDigit(network);
            digits[i] = guessedDigit;
        }
    }
    return digits;
}

int SolveImage(char* path, struct Network* network){
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

    int errorCode = SolveGrayScale(network, grayScale);
    MatDestroy(grayScale);

    if (errorCode != 0){
        printf("Error: SolveImage, SolveGrayScale returned %d\n", errorCode);
        return -3;
    }

    int guessedDigit = GetGuessedDigit(network);

    return guessedDigit;
}

int SolveSudoku(struct OCR* ocr){
    if (ocr == NULL){
        printf("Error: SolveSudoku, invalid argument\n");
        return 1;
    }
    SDL_Surface* sudokuSurface = ocr->settings->sudokuSurface;
    struct Mat* gridGrayScale = ConvertSurfaceToGrayScaleMatrix(sudokuSurface);

    if (gridGrayScale == NULL){
        printf("Error: SolveSudoku, ConvertSurfaceToGrayScaleMatrix is NULL\n");
        return 2;
    }

    size_t horizontalLineCount = 0;
    SDL_Rect* horizontalLines = ScanHorizontalLines(gridGrayScale, &horizontalLineCount);
    size_t horizontalBlockCount = 0;
    SDL_Rect* horizontalBlocks = ConvertHorizontalLinesToBlocks(horizontalLines, horizontalLineCount, &horizontalBlockCount); // this function destroys horizontalLines

    size_t verticalLineCount = 0;
    SDL_Rect* verticalLines = ScanVerticalLines(gridGrayScale, &verticalLineCount);
    size_t verticalBlockCount = 0;
    SDL_Rect* verticalBlocks = ConvertVerticalLinesToBlocks(verticalLines, verticalLineCount, &verticalBlockCount); // this function destroys verticalLines

    free(horizontalLines);
    free(verticalLines);

    SDL_Texture** digitTextures = NULL;
    SDL_Rect* digitRects = NULL;
    struct Mat** digitGrayScales = NULL;
    int* digits = NULL;


    int errorCode = SortBlocks(&horizontalBlocks, &verticalBlocks, &horizontalBlockCount, &verticalBlockCount);
    if (errorCode != 0){
        printf("Error: SolveSudoku, SortBlocks returned %d\n", errorCode);
        goto cleanup;
    }

    digitRects = GetSudokuDigitRects(horizontalBlocks, verticalBlocks); // always return a 81 array
    if (digitRects == NULL){
        printf("Error: SolveSudoku, GetSudokuDigitRects returned NULL\n");
        errorCode = 3;
        goto cleanup;
    }
    digitTextures = GetSudokuDigitTextures(digitRects, sudokuPath);
    if (digitTextures == NULL){
        printf("Error: SolveSudoku, GetSudokuDigitTextures returned NULL\n");
        errorCode = 3;
        goto cleanup;
    }
    digitGrayScales = ConvertTexturesToGrayScale(digitTextures, 81);
    digitGrayScales = DeleteBlankGrayScales(digitGrayScales, 81);

    if (digitGrayScales == NULL){
        printf("Error: SolveSudoku, ConvertTexturesToGrayScale returned NULL\n");
        errorCode = 3;
        goto cleanup;
    }

    digits = SolveGrayScales(digitGrayScales, 81, ocr->network);
    if (digits == NULL){
        printf("Error: SolveSudoku, SolveGrayScales returned NULL\n");
        errorCode = 3;
        goto cleanup;
    }

    ocr->sudoku->horizontalBlocks = horizontalBlocks;
    ocr->sudoku->verticalBlocks = verticalBlocks;
    ocr->sudoku->horizontalBlockCount = horizontalBlockCount;
    ocr->sudoku->verticalBlockCount = verticalBlockCount;
    ocr->sudoku->digitRects = digitRects;
    ocr->sudoku->digitTextures = digitTextures;
    ocr->sudoku->digitGrayScales = digitGrayScales;
    ocr->sudoku->digits = digits;

    cleanup:
    // destroy grid
    MatDestroy(gridGrayScale);
    if (errorCode != 0)
        DestroySudokuArguments(horizontalBlocks, verticalBlocks, digitRects, digitTextures, digitGrayScales, digits);
    return errorCode;


}

struct Network* LoadNetwork(char* path){
    if (path == NULL){
        printf("Error: LoadNetwork, invalid argument\n");
        return NULL;
    }
    FILE* file = fopen(path, "rb");
    if (file == NULL){
        printf("Error: LoadNetwork, impossible to open the file \"%s\"\n", path);
        return NULL;
    }

    size_t layerCount = 0;

    if (!fread(&layerCount, sizeof(size_t), 1, file)){
        printf("Error: LoadNetwork, impossible to read the layerCount number\n");
        fclose(file);
        return NULL;
    }

    struct Mat** weights = malloc(layerCount * sizeof(struct Mat*));
    struct Mat** biases = malloc(layerCount * sizeof(struct Mat*));

    for (size_t i = 0; i < layerCount - 1; i++){ // the last layer is not in the save file because its weights and biases are not usefull and are always recreated
        weights[i] = LoadMatrix(file);
        biases[i] = LoadMatrix(file);
        if (weights[i] == NULL || biases[i] == NULL){
            printf("Error: LoadMatrix returned NULL\n");
            free(weights);
            free(biases);
            fclose(file);
            return NULL;
        }
    }
    fclose(file);
    struct Network* network = CreateNetwork(layerCount, NULL, weights, biases);
    // we don't use the array but we use what they point
    free(weights);
    free(biases);
    if (network == NULL)
        printf("Error: LoadNetwork, CreateNetwork returned NULL\n");
    return network;
}

int SaveNetwork(struct Network* network, char* path){
    if (network == NULL || path == NULL){
        printf("Error: SaveNetwork, invalid argument\n");
        return 1;
    }
    FILE* file = fopen(path, "wb");
    if (file == NULL){
        printf("Error: SaveNetwork, impossible to open the file \"%s\"\n", path);
        return 2;
    }
    if (!fwrite(&network->layerCount, sizeof(size_t), 1, file)){
        printf("Error: SaveNetwork, impossible to write the layerCount in %s\n", path);
        fclose(file);
        return 3;
    }

    for (size_t i = 0; i < network->layerCount-1; i++){ // we don't save the last layer's weights and biases since they are not really used by the network
        struct Layer* currentLayer = network->layers[i];
        int err = 0;
        err = SaveMatrix(currentLayer->weights, file);
        if (err != 0){
            printf("Error: SaveMatrix returned NULL while trying to save the weights in %s\n", path);
            return 5;
        }
        err = SaveMatrix(currentLayer->biases, file);
        if (err != 0){
            printf("Error: SaveMatrix returned NULL while trying to save the biases in %s\n", path);
            return 6;
        }
    }
    fclose(file);
    return 0;
}
