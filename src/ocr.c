#include "header/ocr.h"
#include "header/settings.h"
#include "header/gui.h"
#include "header/neurons.h"
#include <stdio.h>
#include <stdlib.h>

void DestroySudoku(struct Sudoku* sudoku){
    printf("DESTROY SUDOKU\n");
    if (sudoku == NULL){
        printf("WARNING: DestroySudoku, no sudoku to destroy (sudoku is NULL)\n");
        return;
    }
    // free blocks
    if (sudoku->horizontalBlocks != NULL)
        free(sudoku->horizontalBlocks);

    if (sudoku->verticalBlocks != NULL)
        free(sudoku->verticalBlocks);

    // free digitRects
    if (sudoku->digitRects != NULL)
        free(sudoku->digitRects);

    // destroy digitTextures
    if (sudoku->digitTextures != NULL){
        for (int i = 0; i < 81; i++)
            SDL_DestroyTexture(sudoku->digitTextures[i]);
        free(sudoku->digitTextures);
    }

    // destroy grayScales
    if (sudoku->digitGrayScales != NULL){
        for (int i = 0; i < 81; i++)
            MatDestroy(sudoku->digitGrayScales[i]);
        free(sudoku->digitGrayScales);
    }
    // free digits
    if (sudoku->digits != NULL)
        free(sudoku->digits);

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

void DestroyOCR(struct OCR* ocr){
    printf("DESTROY OCR\n");
    if (ocr == NULL){
        printf("WARNING: DestroyOCR, no ocr to destroy (ocr is NULL)\n");
        return;
    }
    DestroySettings(ocr->settings);
    DestroySudoku(ocr->sudoku);
    DestroyNetwork(ocr->network);
    free(ocr);
}

struct Settings* CreateDefaultSettings(){
    struct Settings* settings = malloc(sizeof(struct Settings));
    *settings = (struct Settings){
        .sudokuPath = {0},
        .networkPath = {0},
        .sudokuTexture = NULL,
        .trainingCycleCount = DEFAULT_TRAINING_CYCLE_COUNT,
        .learningRate = DEFAULT_LEARNING_RATE,
        .layerCount = DEFAULT_LAYER_COUNT,
        .neuronsPerLayer = DEFAULT_NEURONS_PER_LAYER
    };

    return settings;
}

struct Sudoku* CreateEmptySudoku(){
    struct Sudoku* sudoku = malloc(sizeof(struct Sudoku));
    *sudoku = (struct Sudoku){0}; // everything is set to NULL or 0
    return sudoku;
}

struct OCR* CreateOCR(){
    struct OCR* ocr = malloc(sizeof(struct OCR));
    ocr->settings = CreateDefaultSettings();
    ocr->sudoku = CreateEmptySudoku();
    ocr->network = NULL; // no network at the beginning, we need to either create it or load it
    return ocr;
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
            nk_rect(50, 50, 300, 200),
            NK_WINDOW_BORDER |
            NK_WINDOW_MOVABLE |
            NK_WINDOW_TITLE))
    {
        nk_layout_row_dynamic(ctx, 30, 1);

        nk_label(ctx, "OCR Settings", NK_TEXT_LEFT);
        if (nk_button_label(ctx, "create network")){
            printf("creating new OCR...\n");
            struct Settings* settings = ocr->settings;
            struct Network* tmp = CreateNetwork(settings->learningRate, settings->layerCount, settings->neuronsPerLayer, NULL, NULL);
            if (tmp == NULL){
                error = 1;
                printf("Error: GuiUpdate, LoadNetwork returned NULL\n");
            }
            else {
                DestroyNetwork(ocr->network);
                ocr->network = tmp;
            }
        }

        if (nk_button_label(ctx, "solve sudoku")){
            printf("Solving Sudoku...\n");
            errorCode = SolveSudoku(ocr);
            if (errorCode != 0){
                error = 1;
                printf("Error: GuiUpdate, SolveSudoku returned an error\n");
            }
        }
        if (nk_button_label(ctx, "train")){
            printf("Training OCR...\n");
            errorCode = TrainButton(ocr);
            if (errorCode != 0){
                error = 1;
                printf("Error non blocking: GuiUpdate, TrainButton returned an error\n");
            }
        }
        if (nk_button_label(ctx, "save")){
            printf("Saving Neural Network...\n");
            errorCode = SaveNetwork(ocr->network, ocr->settings->networkPath);
            if (errorCode != 0){
                error = 1;
                printf("Error: GuiUpdate, SaveNetwork returned an error\n");
            }
        }
        if (nk_button_label(ctx, "load")){
            printf("loading Neural Network...\n");
            struct Network* tmp = LoadNetwork(ocr->settings->networkPath);
            if (tmp == NULL){
                error = 1;
                printf("Error: GuiUpdate, LoadNetwork returned NULL\n");
            }
            else {
                DestroyNetwork(ocr->network);
                ocr->network = tmp;
            }
        }

        nk_edit_string_zero_terminated(
            ctx,
            NK_EDIT_FIELD,
            ocr->settings->networkPath,
            NETWORK_PATH_BUFFER_SIZE,
            nk_filter_float
        );
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
