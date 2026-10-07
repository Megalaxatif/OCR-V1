#pragma once
#include "init.h"
#include "settings.h"

struct Layer{
    struct Mat* weights;       // W
    struct Mat* biases;        // B
    struct Mat* preActivation; // Z
    struct Mat* activation;    // A
};

struct Network{
    size_t layerCount;
    struct Layer** layers;
};

struct Sudoku {
    size_t horizontalBlockCount;
    size_t verticalBlockCount;
    SDL_Rect* verticalBlocks;
    SDL_Rect* horizontalBlocks;
    struct SDL_Rect* digitRects;
    SDL_Texture** digitTextures;
    struct Mat** digitGrayScales;
    int* digits;
};

struct Settings {
    char networkPath[NETWORK_PATH_BUFFER_SIZE];
    int displayDebugInfo;
    SDL_Texture* sudokuTexture;
    SDL_Surface* sudokuSurface;
    size_t trainingCycleCount;
    double learningRate;
    size_t layerCount;
    char neuronsPerLayer[MAXIMUM_LAYER_COUNT][8];
};

struct Info {
    double correctCounter;
    double counter;
};

struct OCR {
    struct Info* info;
    struct Settings* settings;
    struct Sudoku* sudoku;
    struct Network* network;
};


int Update(struct nk_context* ctx, struct OCR* ocr);
void DestroyOCR(struct OCR* ocr);
// this function exists because I'm too lazy to copy paste the same code for both DestroyOCR and the cleanup label of SolveSudoku
void DestroySudokuArguments(
    SDL_Rect* horizontalBlocks,
    SDL_Rect* verticalBlocks,
    SDL_Rect* digitRects,
    SDL_Texture** digitTextures,
    struct Mat** digitGrayScales,
    int* digits);
struct OCR* CreateOCR();
