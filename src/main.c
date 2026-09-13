#include "header/files.h"
#include "header/image.h"
#include "header/math.h"
#include "header/init.h"
#include "header/neurons.h"
#include <SDL2/SDL_render.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>

int errorCode = 0;
size_t* fileCount = NULL; // fileCount[i] correspond to the number of elements in files[i]
int neuronsPerLayer[] = {NETWORK_IMG_SIZE*NETWORK_IMG_SIZE, 256, 128, 10};
struct Network* network = NULL;
struct Mat** answer10 = NULL;
char*** files = NULL;
int* digits = NULL;
char* sudokuPath = "/home/megalaxatif/Documents/code/OCR-V1/sudoku2.png";

time_t seed = 14554;
int main(){
    srand(time(&seed)); // initialise the seed
    InitSDL();

    char** sample10 = malloc(10 * sizeof(char*));
    for(int i = 0; i < 10; i++){
        sample10[i] = malloc(100 * sizeof(char));
    }
    answer10 = GetAnswer10();

    network = CreateNetwork(0.02, 4, neuronsPerLayer, NULL, NULL);
    if (network == NULL){
        printf("Error: main, network is NULL\n");
        errorCode = 1;
        goto cleanup;
    }

    files = GetAllTrainingFileNames(&fileCount);
    if (files == NULL){
        printf("Error: main, files is NULL\n");
        errorCode = 3;
        goto cleanup;
    }

    SDL_Event event;
    int running = 1; // bool
    int trainingCycle = 0;

    while (running){
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT)
                running = 0;
        }
        errorCode = GetSample10(&sample10, files, fileCount);
        if (errorCode != 0){
            printf("Error: main, GetSample10 returned %d\n", errorCode);
            goto cleanup;
        }
        // train with the sample
        Train(network, sample10, 10, answer10);
        trainingCycle++;
        if (trainingCycle == 1500)
            running = 0;
    }

    SDL_SetRenderDrawColor(renderer, 255,0,255,255); // debug
    SDL_SetRenderTarget(renderer, NULL);// debug
    SDL_RenderClear(renderer);// debug

    digits = SolveSudoku(sudokuPath, network);

    int digit0 = SolveImage("/home/megalaxatif/Documents/code/OCR-V1/database/train5/0/0255.png", network,0);
    int digit1 = SolveImage("/home/megalaxatif/Documents/code/OCR-V1/database/train5/1/0255.png", network,0);
    int digit2 = SolveImage("/home/megalaxatif/Documents/code/OCR-V1/database/train5/2/0255.png", network,0);
    int digit3 = SolveImage("/home/megalaxatif/Documents/code/OCR-V1/database/train5/3/0255.png", network,0);
    int digit4 = SolveImage("/home/megalaxatif/Documents/code/OCR-V1/database/train5/4/0255.png", network,0);
    int digit5 = SolveImage("/home/megalaxatif/Documents/code/OCR-V1/database/train5/5/0255.png", network,0);
    int digit6 = SolveImage("/home/megalaxatif/Documents/code/OCR-V1/database/train5/6/0255.png", network,0);
    int digit7 = SolveImage("/home/megalaxatif/Documents/code/OCR-V1/database/train5/7/0255.png", network,0);
    int digit8 = SolveImage("/home/megalaxatif/Documents/code/OCR-V1/database/train5/8/0255.png", network,0);
    int digit9 = SolveImage("/home/megalaxatif/Documents/code/OCR-V1/database/train5/9/0255.png", network,0);

    printf("0 : %d\n", digit0);
    printf("1 : %d\n", digit1);
    printf("2 : %d\n", digit2);
    printf("3 : %d\n", digit3);
    printf("4 : %d\n", digit4);
    printf("5 : %d\n", digit5);
    printf("6 : %d\n", digit6);
    printf("7 : %d\n", digit7);
    printf("8 : %d\n", digit8);
    printf("9 : %d\n", digit9);



    SDL_RenderPresent(renderer); // debug

    if (digits == NULL){
        printf("Error: main, SolveSudoku returned NULL\n");
        goto cleanup;
    }
    //print digits
    printf("\n");
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            int currentDigit = digits[i*9+j];
            currentDigit > 0 ? printf("%d ", currentDigit) : printf("  ");
        }
        printf("\n");
    }
    //free digits
    free(digits);
    running = 1;
    while (running){
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT)
                running = 0;
        }
    }
    //---------------------
    cleanup:
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

    // clean sdl
    DestroySDL();

    //printf("matCount : %ld\n", matCount);
    printf("return code: %d\n", errorCode);
    return errorCode;
}
