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
struct Mat** digitGrayScales = NULL;
struct SDL_Rect** digitRects = NULL;
SDL_Texture** digitTextures = NULL;

int main(){
    srand(time(NULL)); // initialise the seed
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
    char* sudokuPath = "/home/megalaxatif/Documents/code/OCR-V1/sudoku2.png";
    struct Mat* gridGrayScale = GetGridGrayScaleMatrix(sudokuPath);
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
        printf("Error: main, SortBlocks returned %d\n", ret);
        errorCode = 4;
        goto cleanup;
    }

    digitRects = GetSudokuDigitRects(horizontalBlocks, verticalBlocks); // always return a 9 by 9 array
    if (digitRects == NULL){
        printf("Error: main, GetSudokuDigitRects returned NULL\n");
        errorCode = 5;
        goto cleanup;
    }

    digitTextures = GetSudokuDigitTextures(digitRects, sudokuPath);
    if (digitTextures == NULL){
        printf("Error: main, GetSudokuDigitTextures returned NULL\n");
        errorCode = 6;
        goto cleanup;
    }

    digitGrayScales = ConvertTexturesToGrayScale(digitTextures, 81);

    if (digitGrayScales == NULL){
        printf("Error: main, ConvertTexturesToGrayScale returned NULL\n");
        errorCode = 6;
        goto cleanup;
    }

    SDL_Event event;
    int running = 1; // bool
    int trainingCycle = 0;

    SDL_SetRenderDrawColor(renderer, 0,0,0,0);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderClear(renderer);

    // render
    // DrawGrayScale(gridGrayScale);
    // DrawFilledRects(horizontalBlocks, horizontalBlockCount, gridGrayScale, (SDL_Color){255, 0, 0, 255});
    // //DrawRects(verticalLines, verticalLineCount);
    // //DrawRects(horizontalLines, horizontalLineCount);
    // DrawFilledRects(verticalBlocks, verticalBlockCount, gridGrayScale, (SDL_Color){255, 0, 0, 255});
    // for(int i = 0; i < 9; i++){
    //     DrawRects(digitRects[i], 9, gridGrayScale, (SDL_Color){0, 225, 0, 255});
    // }
    // DrawDigitGrayScales(digitGrayScales, digitRects);
    for(int k = 0; k < 9; k++){
        for(int l = 0; l < 9; l++){
            SDL_RenderCopy(renderer, digitTextures[k*9+l], NULL, digitRects[k] + l);
        }
    }
    SDL_RenderPresent(renderer);

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
        if (trainingCycle == 150)
            running = 0;
    }

    digits = SolveGrayScale(digitGrayScales, 81, network);
    if (digits == NULL){
        printf("Error: main, SolveGrayScale returned NULL\n");
        errorCode = 6;
        goto cleanup;
    }
    //print the array
    printf("\n");
    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            printf("%d ", digits[i*9+j]);
        }
        printf("\n");
    }
    while(1){
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT)
                break;
        }
    }
    cleanup:
    MatDestroy(gridGrayScale);
    if (digitGrayScales != NULL){
        for (int i = 0; i < 81; i++){
            MatDestroy(digitGrayScales[i]);
        }
        network->layers[0]->activation = NULL; // important
    }
    // clean network
    DestroyNetwork(network);

    free(horizontalLines);
    free(horizontalBlocks);
    free(verticalLines);
    free(verticalBlocks);

    // clean digits
    free(digits);

    // clean digitRects
    if (digitRects != NULL){
        for(int i = 0; i < 9; i++){
            free(digitRects[i]);
        }
        free(digitRects);
    }

    // free digitTextures
    if (digitTextures != NULL){
        for (int i = 0; i < 81; i++)
            SDL_DestroyTexture(digitTextures[i]);
        free(digitTextures);
    }
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
