#include "header/files.h"
#include "header/image.h"
#include "header/math.h"
#include "header/init.h"
#include "header/neurons.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>

int errorCode = 0;
size_t* fileCount = NULL; // fileCount[i] correspond to the number of elements in files[i]
int neuronsPerLayer[] = {NETWORK_IMG_SIZE*NETWORK_IMG_SIZE, 16, 10};
struct Network* network = NULL;
struct Mat** answer10 = NULL;
char*** files = NULL;

int main(){
    srand(time(NULL)); // initialise the seed
    InitSDL();

    char** sample10 = malloc(10 * sizeof(char*));
    for(int i = 0; i < 10; i++){
        sample10[i] = malloc(100 * sizeof(char));
    }

    network = CreateNetwork(0.02, 3, neuronsPerLayer, NULL, NULL);
    if (network == NULL){
        printf("Error: main, network is NULL\n");
        errorCode = 1;
        goto cleanup;
    }
    answer10 = GetAnswer10();
    if (answer10 == NULL){
        printf("Error: main, answer10 is NULL\n");
        errorCode = 2;
        goto cleanup;
    }
    files = GetAllTrainingFileNames(&fileCount);
    if (files == NULL){
        printf("Error: main, files is NULL\n");
        errorCode = 3;
        goto cleanup;
    }

    SDL_Event event;
    int running = 1;

    struct Mat* grayScale = GetGrayScaleMatrix("/home/megalaxatif/Documents/code/OCR/test grid.png");
    while (running){
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT)
                running = 0;
        }

        // errorCode = GetSample10(&sample10, files, fileCount);
        // if (errorCode != 0){
        //     printf("Error: main, GetSample10 returned %d\n", errorCode);
        //     goto cleanup;
        // }
        // // train with the sample
        // Train(network, sample10, 10, answer10);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderClear(renderer);

        // render
        DrawGrayScale(grayScale);

        SDL_RenderPresent(renderer);
        SDL_Delay(100); // delay to limit the frame rate
    }
    cleanup:
    MatDestroy(grayScale);
    // clean sample
    if (sample10 != NULL){
        for(int i = 0; i < 10; i++){
            free(sample10[i]);
        }
        free(sample10);
    }

    // clean answer
    if (answer10 != NULL){
        for(int i = 0; i < 10; i++){
            MatDestroy(answer10[i]);
        }
        free(answer10);
    }

    // clean files
    if (files != NULL){
        for(size_t i = 0; i < 10; i++){
            for(size_t j = 0; j < fileCount[i]; j++){
                free(files[i][j]);
            }
            free(files[i]);
        }
        free(files);
    }

    // clean fileCount
    if (fileCount != NULL){
        free(fileCount);
    }

    // clean network and sdl
    DestroyNetwork(network);
    DestroySDL();

    printf("matCount : %ld\n", matCount);

    printf("return code: %d\n", errorCode);
    return errorCode;
}
