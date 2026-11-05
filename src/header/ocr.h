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

struct GridInfo {
    size_t horizontalBlockCount;
    size_t verticalBlockCount;
    SDL_Rect* verticalBlocks;
    SDL_Rect* horizontalBlocks;
    struct SDL_Rect* letterRects;
    int* letters;
};

struct Settings {
    char networkPath[NETWORK_PATH_BUFFER_SIZE];
    int displayDebugInfo;
    SDL_Texture* gridTexture;
    SDL_Surface* gridSurface;
    struct Mat* gridMatrix;
    size_t trainingCycleCount;
    double learningRate;
    size_t layerCount;
    char neuronsPerLayer[MAXIMUM_LAYER_COUNT][8];
};
// TODO: maybe put this in Settings
struct Info {
    double correctCounter;
    double counter;
};

struct OCR {
    struct Info* info;
    struct Settings* settings;
    struct GridInfo* gridInfo;
    struct Network* network;
};


int Update(struct nk_context* ctx, struct OCR* ocr);
struct OCR* CreateOCR();
void DestroyOCR(struct OCR* ocr);
// this function exists because I'm too lazy to copy paste the same code for both DestroyOCR and the cleanup label of SolveGrid
void DestroyGridArguments(
    SDL_Rect* horizontalBlocks,
    SDL_Rect* verticalBlocks,
    SDL_Rect* letterRects,
    int* letters);
struct GridInfo* CreateEmptyGrid();
void DestroyGrid(struct GridInfo* grid);

void ChangeGrid(struct OCR* ocr, SDL_Surface* newGrid);
