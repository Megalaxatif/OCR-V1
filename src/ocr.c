#include "header/ocr.h"
#include "header/settings.h"
#include "header/gui.h"
#include "header/neurons.h"
#include <SDL2/SDL_stdinc.h>
#include <stdio.h>
#include <stdlib.h>

void DestroySudokuArguments(
    SDL_Rect* horizontalBlocks,
    SDL_Rect* verticalBlocks,
    SDL_Rect* digitRects,
    SDL_Texture** digitTextures,
    struct Mat** digitGrayScales,
    int* digits){

    // free blocks
    if (horizontalBlocks != NULL)
        free(horizontalBlocks);

    if (verticalBlocks != NULL)
        free(verticalBlocks);

    // free digitRects
    if (digitRects != NULL)
        free(digitRects);

    // destroy digitTextures
    if (digitTextures != NULL){
        for (int i = 0; i < 81; i++)
            SDL_DestroyTexture(digitTextures[i]);
        free(digitTextures);
    }

    // destroy grayScales
    if (digitGrayScales != NULL){
        for (int i = 0; i < 81; i++)
            MatDestroy(digitGrayScales[i]);
        free(digitGrayScales);
    }
    // free digits
    if (digits != NULL)
        free(digits);
}

void DestroySudoku(struct Sudoku* sudoku){
    printf("DESTROY SUDOKU\n");
    if (sudoku == NULL){
        printf("WARNING: DestroySudoku, no sudoku to destroy (sudoku is NULL)\n");
        return;
    }
    DestroySudokuArguments(
        sudoku->horizontalBlocks,
        sudoku->verticalBlocks,
        sudoku->digitRects,
        sudoku->digitTextures,
        sudoku->digitGrayScales,
        sudoku->digits
    );

    free(sudoku);
}

void DestroySettings(struct Settings* settings){
    printf("DESTROY SETTINGS\n");
    if (settings == NULL){
        printf("WARNING: DestroySettings, no settings to destroy (settings is NULL)\n");
        return;
    }
    if (settings->sudokuTexture != NULL)
        SDL_DestroyTexture(settings->sudokuTexture);
    free(settings);
}

struct Settings* CreateDefaultSettings(){
    struct Settings* settings = malloc(sizeof(struct Settings));
    *settings = (struct Settings){
        .sudokuPath = {0},
        .networkPath = DEFAULT_NETWORK_PATH,
        .displayDebugInfo = DEFAULT_DEBUG_INFO,
        .sudokuTexture = NULL,
        .trainingCycleCount = DEFAULT_TRAINING_CYCLE_COUNT,
        .learningRate = DEFAULT_LEARNING_RATE,
        .layerCount = DEFAULT_LAYER_COUNT,
        .neuronsPerLayer = {{0}}
    };
    // convert the int array into a str array
    int neuronsPerLayer[] = DEFAULT_NEURONS_PER_LAYER;
    for (int i = 0; i < DEFAULT_LAYER_COUNT; i++){
        snprintf(settings->neuronsPerLayer[i], sizeof(settings->neuronsPerLayer[i]), "%d", neuronsPerLayer[i]);
    }
    return settings;
}

struct Info* CreateInfo(){
    struct Info* info = malloc(sizeof(struct Info));
    info->correctCounter = 0;
    info->counter = 0;
    return info;
}

void DestroyInfo(struct Info* info){
    free(info);
}

struct Sudoku* CreateEmptySudoku(){
    struct Sudoku* sudoku = malloc(sizeof(struct Sudoku));
    *sudoku = (struct Sudoku){0}; // everything is set to NULL or 0
    return sudoku;
}

struct OCR* CreateOCR(){
    struct OCR* ocr = malloc(sizeof(struct OCR));
    ocr->info = CreateInfo();
    ocr->settings = CreateDefaultSettings();
    ocr->sudoku = CreateEmptySudoku();
    ocr->network = CreateNetwork(DEFAULT_LEARNING_RATE, DEFAULT_LAYER_COUNT, (int[])DEFAULT_NEURONS_PER_LAYER, NULL, NULL);
    return ocr;
}

void DestroyOCR(struct OCR* ocr){
    printf("DESTROY OCR\n");
    if (ocr == NULL){
        printf("WARNING: DestroyOCR, no ocr to destroy (ocr is NULL)\n");
        return;
    }
    DestroyInfo(ocr->info);
    DestroySettings(ocr->settings);
    DestroySudoku(ocr->sudoku);
    DestroyNetwork(ocr->network);
    free(ocr);
}

int GuiUpdate(struct nk_context* ctx, struct OCR* ocr){
    if (ctx == NULL || ocr == NULL) {
        printf("Error: GuiUpdate, invalid argument\n");
        return 1;
    }
    int errorCode = 0;
    int error = 0;
    if (nk_begin(
            ctx,
            "OCR Settings",
            nk_rect(50, 50, 420, 430),
            NK_WINDOW_BORDER |
            NK_WINDOW_MOVABLE |
            NK_WINDOW_TITLE))
    {
        //NETWORK
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label(ctx, "Neural Network", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 2);

        if (nk_button_label(ctx, "Create")){
            printf("Creating new network...\n");
            errorCode = CreateNetworkButton(ocr);
            if (errorCode != 0){
                error = 1;
                printf("Error: GuiUpdate, CreateNetworkButton returned an error\n");
            }
        }
        if (nk_button_label(ctx, "Load")){
            printf("Loading Neural Network...\n");
            errorCode = LoadNetworkButton(ocr);
            if (errorCode != 0){
                error = 1;
                printf("Error: GuiUpdate, LoadNetworkButton returned an error\n");
            }
        }
        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_button_label(ctx, "Save Network")){
            printf("Saving Neural Network...\n");
            errorCode = SaveNetwork(ocr->network, ocr->settings->networkPath);
            if (errorCode != 0){
                error = 1;
                printf("Error: GuiUpdate, SaveNetwork returned an error\n");
            }
        }
        // NETWORK PATH

        nk_label(ctx, "Network path", NK_TEXT_LEFT);

        nk_edit_string_zero_terminated(
            ctx,
            NK_EDIT_FIELD,
            ocr->settings->networkPath,
            NETWORK_PATH_BUFFER_SIZE,
            nk_filter_default
        );

        //TRAINING
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label(ctx, "Training", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 35, 1);

        if (nk_button_label(ctx, "Train Network")){
            printf("Training Network...\n");
            errorCode = TrainButton(ocr);
            if (errorCode != 0){
                error = 1;
                printf("Error: GuiUpdate, TrainButton returned an error\n");
            }
        }

        //SUDOKU
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label(ctx, "Sudoku", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 35, 1);

        if (nk_button_label(ctx, "Solve Sudoku")){
            printf("Solving Sudoku...\n");
            errorCode = SolveSudoku(ocr);
            if (errorCode != 0){
                error = 1;
                printf("Error: GuiUpdate, SolveSudoku returned an error\n");
            }
        }

        //OPTIONS
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label(ctx, "Options", NK_TEXT_LEFT);
        nk_checkbox_label(ctx, "Display debug info", &ocr->settings->displayDebugInfo);

        LearningRateInterface(ocr, ctx);
        NeuronsPerLayerInterface(ocr, ctx);

    }
    else {
        error = 1;
        printf("Error: GuiUpdate, nk_begin failed\n");
    }
    nk_end(ctx);
    return error;
}


int Update(struct nk_context* ctx, struct OCR* ocr){
    SDL_Event event;
    nk_input_begin(ctx);
    while (SDL_PollEvent(&event)){
        if (event.type == SDL_QUIT)
            return -1;
        nk_sdl_handle_event(&event);

        if (event.type == SDL_DROPFILE){
            // TODO check if the file has a valid extension
            char* tmp = event.drop.file;
            size_t len = strlen(tmp);
            if (len >= SUDOKU_PATH_BUFFER_SIZE){
                printf("Error: Update, the path given is %ld bytes long but the maximum size allowed is %d bytes\n", len+1, SUDOKU_PATH_BUFFER_SIZE);
                SDL_free(tmp);
                continue;
            }
            strcpy(ocr->settings->sudokuPath, tmp);
            SDL_free(tmp);
            if (ocr->settings->sudokuTexture != NULL)
                SDL_DestroyTexture(ocr->settings->sudokuTexture);
            SDL_Surface* sudokuSurface = IMG_Load(ocr->settings->sudokuPath);
            ocr->settings->sudokuTexture = SDL_CreateTextureFromSurface(renderer, sudokuSurface);
            SDL_FreeSurface(sudokuSurface);
            printf("file droped : %s\n", ocr->settings->sudokuPath);
        }
    }
    nk_input_end(ctx);
    // ----- Render -----
    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    int errorCode = GuiUpdate(ctx, ocr);
    if (errorCode != 0)
        printf("Error non blocking: Update, GuiUpdate returned %d\n", errorCode);
    SDL_SetRenderTarget(renderer, NULL);

    if (ocr->settings->sudokuTexture)
        SDL_RenderCopy(renderer, ocr->settings->sudokuTexture, NULL, NULL);

    nk_sdl_render(NK_ANTI_ALIASING_ON);
    SDL_RenderPresent(renderer);
    return errorCode;
}
