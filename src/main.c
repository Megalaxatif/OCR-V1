#include "header/files.h"
#include "header/math.h"
#include "header/init.h"
#include "header/image.h"
#include "header/neurons.h"
#include <string.h>

int errorCode = 0;
size_t* fileCount = NULL; // fileCount[i] correspond to the number of elements in files[i]
int neuronsPerLayer[] = {NETWORK_IMG_SIZE*NETWORK_IMG_SIZE, 16, 16, 10};
struct Network* network = NULL;
struct Mat** answer10 = NULL;
char*** files = NULL;

int main(){
    InitSDL();
    network = CreateNetwork(0.03, 4, neuronsPerLayer, NULL, NULL);
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

    while (running){
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT)
                running = 0;
        }
        // create the sample
        char* sample[10];
        for(int i = 0; i < 10; i++){
            size_t index = Random(0, fileCount[i]-1);
            sample[i] = files[i][index];
        }

        // train with the sample
        Train(network, sample, SAMPLE_SIZE, answer10);
        // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        // SDL_SetRenderTarget(renderer, NULL);
        // SDL_RenderClear(renderer);

        // SDL_RenderPresent(renderer);
        // SDL_Delay(100); // delay to limit the frame rate
    }
    cleanup:

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

    if (fileCount != NULL){
        free(fileCount);
    }

    DestroyNetwork(network);
    DestroySDL();
    printf("return code: %d\n", errorCode);
    return errorCode;
}
