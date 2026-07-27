#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#define SCREEN_W 1000
#define SCREEN_H 1000
#define NETWORK_IMG_SIZE 28 // pixel width and height of the images that the network can take
//#define TRAIN_DIRECTORY_PATH "../train/" // path where the train directory containing all the training images is in the project
#define TRAIN_DIRECTORY_PATH "/home/megalaxatif/Documents/code/OCR/train/"

extern size_t matCount;

extern SDL_Renderer* renderer;
extern SDL_Window* window;

int InitSDL();
void DestroySDL();
