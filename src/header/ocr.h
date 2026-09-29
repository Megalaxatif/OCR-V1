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
    double learningRate;
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
    char sudokuPath[SUDOKU_PATH_BUFFER_SIZE];
    char networkPath[NETWORK_PATH_BUFFER_SIZE];
    SDL_Texture* sudokuTexture;
    size_t trainingCycleCount;
    double learningRate;
    size_t layerCount;
    int neuronsPerLayer[MAXIMUM_LAYER_COUNT];
};

struct OCR {
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
