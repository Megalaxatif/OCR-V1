#pragma once
// SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>

// nuklear
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include "../../lib/nuklear/nuklear.h"
#include "../../lib/nuklear/nuklear_sdl_renderer.h"

// OCR
#define SCREEN_W 1000
#define SCREEN_H 1000
#define NETWORK_IMG_SIZE 28 // pixel width and height of the images that the network can take
#define GRID_OFFSET 3 // the rectangle of a digit in the grid might include part of the grid so we add this offset on each side of the rectangle to prevent that

#define MATRIX_BLANKNESS_THRESHOLD 0.90 // percentage of values close to 1 we must have in a matrix to say that it's blank
#define MATRIX_VALUE_BLANKNESS_THRESHOLD 0.95 // considering that every value in the matrix is between 0 and 1, if a pixel has a value superior to that number we consider it as a 1 (blank)
#define TRAIN_DIRECTORY_PATH "/home/megalaxatif/Documents/code/OCR-V1/database/train5/" // path where the train directory containing all the training images is in the project

extern size_t matCount; // TODO: remove

extern SDL_Renderer* renderer;
extern SDL_Window* window;

int InitSDL();
void DestroySDL();
