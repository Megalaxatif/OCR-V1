#include "header/files.h"
#include "header/image.h"
#include "header/math.h"
#include "header/init.h"
#include "header/neurons.h"
#include <SDL2/SDL_render.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>

int errorCode = 0;
int targetTrainingCycle = 15;
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
    // nuklear
    struct nk_context *ctx = nk_sdl_init(window, renderer);
    struct nk_font_atlas *atlas;

    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();
    //------

    char** sample10 = malloc(10 * sizeof(char*));
    for(int i = 0; i < 10; i++){
        sample10[i] = malloc(100 * sizeof(char));
    }
    answer10 = GetAnswer10();
    if (answer10 == NULL){
        printf("Error: main, answer10 is NULL\n");
        errorCode = 1;
        goto cleanup;
    }
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

    while (running){
        nk_input_begin(ctx);
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT)
                running = 0;
            nk_sdl_handle_event(&event);
        }
        nk_input_end(ctx);

        // nuklear
        if (nk_begin(
                ctx,
                "Fenetre",
                nk_rect(50, 50, 300, 200),
                NK_WINDOW_BORDER |
                NK_WINDOW_MOVABLE |
                NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(ctx, 30, 1);

            nk_label(ctx, "Salut Nuklear", NK_TEXT_LEFT);

            if (nk_button_label(ctx, "solve sudoku")){
                digits = SolveSudoku(sudokuPath, network);
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
            }
            if (nk_button_label(ctx, "train")){
                int trainingCycle = 0;
                while (trainingCycle < targetTrainingCycle){
                    errorCode = GetSample10(&sample10, files, fileCount);
                    if (errorCode != 0){
                        printf("Error: main, GetSample10 returned %d\n", errorCode);
                        goto cleanup;
                    }
                    // train with the sample
                    Train(network, sample10, 10, answer10);
                    trainingCycle++;
                }
            }
            if (nk_button_label(ctx, "save")){
                printf("saving OCR...\n");
                int err = SaveNetwork(network, "save.ocr");
                if (err != 0){
                    printf("Error: main, SaveNetwork returned an error\n");
                    errorCode = 1;
                    goto cleanup;
                }

            }
            if (nk_button_label(ctx, "load")){
                printf("loading OCR...\n");
                struct Network* tmp = LoadNetwork("save.ocr");
                if (tmp == NULL){
                    printf("Error: main, LoadNetwork returned NULL\n");
                    errorCode = 1;
                    goto cleanup;
                }
                DestroyNetwork(network);
                network = tmp;
            }
        }
        nk_end(ctx);

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        nk_sdl_render(NK_ANTI_ALIASING_ON);

        SDL_RenderPresent(renderer);
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

    DestroyNetwork(network);

    // clean sdl
    DestroySDL();

    // clean nuklear
    nk_sdl_shutdown();

    //printf("matCount : %ld\n", matCount);
    printf("return code: %d\n", errorCode);
    return errorCode;
}
