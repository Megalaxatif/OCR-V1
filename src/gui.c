#include "header/init.h"
#include "header/math.h"
#include "header/neurons.h"
#include "header/files.h"
#include "header/ocr.h"
#include "header/settings.h"
#include <stdio.h>
#include <string.h>

void LearningRateInterface(struct OCR* ocr, struct nk_context* ctx){
    char str[64] = {0};
    double learningRate = ocr->settings->learningRate;
    snprintf(str, sizeof(str), "learning rate: %.3f", learningRate);
    nk_label(ctx, str, NK_TEXT_LEFT);

    float sliderValue = (float)learningRate;
    nk_slider_float(ctx, 0.001, &sliderValue, 0.1, 0.001);
    ocr->settings->learningRate = (double)sliderValue;
}

void NeuronsPerLayerInterface(struct OCR* ocr, struct nk_context* ctx){
    size_t layerCount = ocr->settings->layerCount;
    nk_layout_row_dynamic(ctx, 25, 2);

    if (nk_button_label(ctx, "Add hidden layer")){
        if (layerCount + 1 <= MAXIMUM_LAYER_COUNT){
            // shift the output layer to the right
            strcpy(ocr->settings->neuronsPerLayer[layerCount], ocr->settings->neuronsPerLayer[layerCount - 1]);
            strcpy(ocr->settings->neuronsPerLayer[layerCount - 1], "0");
            ocr->settings->layerCount ++;
        }
    }
    if (nk_button_label(ctx, "Remove hidden layer")){
        if(layerCount - 1 >= DEFAULT_LAYER_COUNT){
            // shift the output layer to the left
            strcpy(ocr->settings->neuronsPerLayer[layerCount - 2], ocr->settings->neuronsPerLayer[layerCount-1]);
            ocr->settings->layerCount --;
        }
    }
    nk_layout_row_dynamic(ctx, 25, 2);
    for(size_t i = 1; i < ocr->settings->layerCount-1; i++){ // we don't want the user to change the neuron number of the first and the last layer
        char str[32] = {0};
        snprintf(str, sizeof(str), "Hidden layer %ld", i);
        nk_label(ctx, str, NK_TEXT_LEFT);
        char* buffer = ocr->settings->neuronsPerLayer[i];
        nk_edit_string_zero_terminated(
            ctx,
            NK_EDIT_FIELD,
            buffer,
            sizeof(buffer),
            nk_filter_decimal
        );
    }
}

int LoadNetworkButton(struct OCR* ocr){
    if (ocr == NULL){
        printf("Error: LoadNetworkButton, invalid argument\n");
        return 1;
    }
    struct Network* tmp = LoadNetwork(ocr->settings->networkPath);
    if (tmp == NULL){
        printf("Error: LoadNetworkButton, LoadNetwork returned NULL\n");
        return 1;
    }
    else {
        DestroyNetwork(ocr->network);
        ocr->network = tmp;
    }
    return 0;
}

int CreateNetworkButton(struct OCR* ocr){
    if (ocr == NULL){
        printf("Error: CreateNetworkButton, invalid argument\n");
        return 1;
    }
    struct Settings* settings = ocr->settings;

    // convert the string array to an int array
    int neuronsPerLayer[MAXIMUM_LAYER_COUNT] = {0};
    for(size_t i = 0; i < settings->layerCount; i++)
        neuronsPerLayer[i] = atoi(settings->neuronsPerLayer[i]);

    struct Network* tmp = CreateNetwork(settings->layerCount, neuronsPerLayer, NULL, NULL);
    if (tmp == NULL){
        printf("Error: CreateNetworkButton, CreateNetwork returned NULL\n");
        return 1;
    }
    else {
        DestroyNetwork(ocr->network);
        ocr->network = tmp;
    }
    return 0;
}

int TrainButton(struct OCR* ocr){
    if (ocr == NULL){
        printf("Error: TrainButton, invalid argument \n");
        return 1;
    }
    // TODO: do this in another processus

    ocr->info->correctCounter = 0;
    ocr->info->counter = 0;

    int errorCode = 0;
    struct Mat** answer10 = NULL;
    char*** files = NULL;
    size_t* fileCount = NULL; // fileCount[i] correspond to the number of elements in files[i]

    char** sample10 = malloc(10 * sizeof(char*));
    for(int i = 0; i < 10; i++)
        sample10[i] = malloc(100 * sizeof(char));

    answer10 = GetAnswer10();
    if (answer10 == NULL){
        printf("Error: ..., GetAnswer10 returned NULL\n");
        errorCode = 2;
        goto cleanup;
    }
    files = GetAllTrainingFileNames(&fileCount);
    if (files == NULL){
        printf("Error: ..., GetAllTrainingFileNames returned NULL\n");
        errorCode = 3;
        goto cleanup;
    }
    size_t currentTrainingCycleCount = 0;
    while (currentTrainingCycleCount < ocr->settings->trainingCycleCount){
        errorCode = GetSample10(&sample10, files, fileCount);
        if (errorCode != 0){
            printf("Error: ..., GetSample10 returned %d\n", errorCode);
            goto cleanup;
        }
        // train with the sample
        errorCode = Train(ocr, sample10, 10, answer10);
        if (errorCode != 0){
            printf("Error: ..., Train returned %d\n", errorCode);
            goto cleanup;
        }
        currentTrainingCycleCount++;
    }
    cleanup:
    // clean sample
    if (sample10 != NULL){
        for(int i = 0; i < 10; i++)
            free(sample10[i]);
        free(sample10);
    }
    // clean answer
    if (answer10 != NULL){
        for(int i = 0; i < 10; i++)
            MatDestroy(answer10[i]);
        free(answer10);
    }
    // clean files
    if (files != NULL){
        for(size_t i = 0; i < 10; i++){
            for(size_t j = 0; j < fileCount[i]; j++)
                free(files[i][j]);
            free(files[i]);
        }
        free(files);
    }
    // clean fileCount
    if (fileCount != NULL)
        free(fileCount);
    return errorCode;
}
