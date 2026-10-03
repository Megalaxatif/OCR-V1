#include "header/math.h"
#include "header/neurons.h"
#include "header/files.h"
#include "header/ocr.h"

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
    struct Network* tmp = CreateNetwork(settings->learningRate, settings->layerCount, settings->neuronsPerLayer, NULL, NULL);
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
    if (ocr->network == NULL){
        printf("Error non blocking: TrainButton, no network to train\n");
        return 1;
    }
    // TODO: do this in another processus
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
        errorCode = Train(ocr->network, sample10, 10, answer10);
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
