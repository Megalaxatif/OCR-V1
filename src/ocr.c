#include "header/ocr.h"
#include "header/gui.h"
#include "header/neurons.h"
#include "header/image.h"
#include <SDL2/SDL_surface.h>
#include <stdio.h>

void DestroyGridArguments(
    SDL_Rect* horizontalBlocks,
    SDL_Rect* verticalBlocks,
    SDL_Rect* letterRects,
    int* letters){

    // free blocks
    if (horizontalBlocks != NULL)
        free(horizontalBlocks);

    if (verticalBlocks != NULL)
        free(verticalBlocks);

    // free letterRects
    if (letterRects != NULL){
        free(letterRects);
    }
    // free letters
    if (letters != NULL)
        free(letters);
}

void DestroyGrid(struct GridInfo* grid){
    printf("DESTROY SUDOKU\n");
    if (grid == NULL){
        printf("WARNING: DestroyGrid, no grid to destroy (grid is NULL)\n");
        return;
    }
    DestroyGridArguments(
        grid->horizontalBlocks,
        grid->verticalBlocks,
        grid->letterRects,
        grid->letters
    );

    free(grid);
}

void DestroySettings(struct Settings* settings){
    printf("DESTROY SETTINGS\n");
    if (settings == NULL){
        printf("WARNING: DestroySettings, no settings to destroy (settings is NULL)\n");
        return;
    }
    if (settings->gridTexture != NULL)
        SDL_DestroyTexture(settings->gridTexture);
    if (settings->gridSurface != NULL)
        SDL_FreeSurface(settings->gridSurface);
    if (settings->gridMatrix != NULL)
        MatDestroy(settings->gridMatrix);
    free(settings);
}

struct Settings* CreateDefaultSettings(){
    struct Settings* settings = malloc(sizeof(struct Settings));
    *settings = (struct Settings){
        .networkPath = DEFAULT_NETWORK_PATH,
        .displayDebugInfo = DEFAULT_DEBUG_INFO,
        .gridTexture = NULL,
        .gridSurface = NULL,
        .gridMatrix = NULL,
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

struct GridInfo* CreateEmptyGrid(){
    struct GridInfo* grid = malloc(sizeof(struct GridInfo));
    *grid = (struct GridInfo){0}; // everything is set to NULL or 0
    return grid;
}

struct OCR* CreateOCR(){
    struct OCR* ocr = malloc(sizeof(struct OCR));
    ocr->info = CreateInfo();
    ocr->settings = CreateDefaultSettings();
    ocr->gridInfo = CreateEmptyGrid();
    ocr->network = CreateNetwork(DEFAULT_LAYER_COUNT, (int[])DEFAULT_NEURONS_PER_LAYER, NULL, NULL);
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
    DestroyGrid(ocr->gridInfo);
    DestroyNetwork(ocr->network);
    free(ocr);
}

void ChangeGrid(struct OCR* ocr, SDL_Surface* newGrid){
    if (newGrid == NULL){
        printf("Error: ChangeGrid, invalid argument\n");
        return;
    }
    // destroy the previous matrix
    if (ocr->settings->gridMatrix != NULL)
        MatDestroy(ocr->settings->gridMatrix);

    //set the new matrix
    ocr->settings->gridMatrix = ConvertSurfaceToGrayScaleMatrix(newGrid);

    // destroy the previous texture
    if (ocr->settings->gridTexture != NULL)
        SDL_DestroyTexture(ocr->settings->gridTexture);

    // set the new texture
    ocr->settings->gridTexture = SDL_CreateTextureFromSurface(renderer, newGrid);

    // destroy the previous surface
    if (ocr->settings->gridSurface != NULL)
        SDL_FreeSurface(ocr->settings->gridSurface);

    // set the new surface
    ocr->settings->gridSurface = newGrid;
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
            nk_rect(50, 50, 250, 700),
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
        nk_label(ctx, "Grid", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 35, 1);

        if (nk_button_label(ctx, "Solve Grid")){
            printf("Solving Grid...\n");
            errorCode = SolveGrid(ocr);
            if (errorCode != 0){
                error = 1;
                printf("Error: GuiUpdate, SolveGrid returned an error\n");
            }
        }
        RotateInterface(ocr, ctx);

        //OPTIONS
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_label(ctx, "Options", NK_TEXT_LEFT);
        nk_checkbox_label(ctx, "Display debug info", &ocr->settings->displayDebugInfo);

        LearningRateInterface(ocr, ctx);
        TrainingCycleInterface(ocr, ctx);
        NeuronsPerLayerInterface(ocr, ctx);
    }
    else {
        error = 1;
        printf("Error: GuiUpdate, nk_begin failed\n");
    }
    nk_end(ctx);
    return error;
}

int HandleDropedFile(SDL_Event event, struct OCR* ocr){
    // TODO check if the file has a valid extension
    char* tmp = event.drop.file;
    SDL_Surface* gridSurface = IMG_Load(tmp);
    SDL_free(tmp);
    if (gridSurface == NULL){
        printf("Error: HandleDropedFile, impossible to load the image\n");
        return 1;
    }
    // get the grayScale
    SDL_Surface* convertedSurface = SDL_ConvertSurfaceFormat(gridSurface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(gridSurface);
    SDL_Surface* grayScale = ConvertSurfaceToGrayScale(convertedSurface);
    if (grayScale == NULL){
        printf("Error: HandleDropedFile, ConvertSurfaceToGrayScale returned NULL\n");
        SDL_FreeSurface(convertedSurface);
        return 1;
    }
    convertedSurface = grayScale;

    // rotate the surface
    SDL_Surface* rotatedSurface = DeskewSurface(convertedSurface);
    SDL_FreeSurface(convertedSurface);
    if (rotatedSurface == NULL) {
        printf("Error: HandleDropedFile, DeskewSurface returned NULL\n");
        SDL_free(tmp);
        return 1;
    }
    ChangeGrid(ocr, rotatedSurface);
    // reset the grid informations
    DestroyGrid(ocr->gridInfo);
    ocr->gridInfo = CreateEmptyGrid();

    return 0;
}


int Update(struct nk_context* ctx, struct OCR* ocr){
    SDL_Event event;
    int errorCode = 0;
    nk_input_begin(ctx);
    while (SDL_PollEvent(&event)){
        if (event.type == SDL_QUIT)
            return -1;
        nk_sdl_handle_event(&event);

        if (event.type == SDL_DROPFILE){
            errorCode = HandleDropedFile(event, ocr);
            if (errorCode != 0)
                printf("Error: HandleDropedFile failed\n");
        }
    }
    nk_input_end(ctx);
    // ----- Render -----
    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    errorCode = GuiUpdate(ctx, ocr);
    if (errorCode != 0)
        printf("Error : Update, GuiUpdate failed\n");
    SDL_SetRenderTarget(renderer, NULL);

    if (ocr->settings->gridTexture)
        SDL_RenderCopy(renderer, ocr->settings->gridTexture, NULL, NULL);

    if (ocr->settings->displayDebugInfo){
        if (ocr->gridInfo->letterRects)
            DrawRects(ocr->gridInfo->letterRects, 81, ocr->settings->gridMatrix, (SDL_Color){255, 0, 0, 255});
            // for (int i = 0; i < 81; i++){
            //     SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            //     SDL_RenderDrawRects(renderer, ocr->gridInfo->letterRects, 81);
            // }
        if (ocr->gridInfo->horizontalBlocks)
            DrawRects(ocr->gridInfo->horizontalBlocks, ocr->gridInfo->horizontalBlockCount, ocr->settings->gridMatrix, (SDL_Color){0, 255, 0, 255});
            // for (int i = 0; i < 81; i++){
            //     SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            //     SDL_RenderFillRects(renderer, ocr->gridInfo->horizontalBlocks, ocr->gridInfo->horizontalBlockCount);
            // }
        if (ocr->gridInfo->verticalBlocks)
            DrawRects(ocr->gridInfo->verticalBlocks, ocr->gridInfo->verticalBlockCount, ocr->settings->gridMatrix, (SDL_Color){0, 255, 0, 255});

        if (ocr->gridInfo->letters){
            for (int i = 0; i <9; i++){
                for (int j = 0; j < 9; j++){
                    int letter = ocr->gridInfo->letters[i*9 +j];
                    if (letter == -1)
                        printf("  ");
                    else
                        printf("%d ", ocr->gridInfo->letters[i*9 +j]);
                }
                printf("\n");
            }
            printf("\n");
        }
    }
    nk_sdl_render(NK_ANTI_ALIASING_ON);
    SDL_RenderPresent(renderer);
    return errorCode;
}
