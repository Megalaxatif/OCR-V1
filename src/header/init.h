#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#define SCREEN_W 1000
#define SCREEN_H 1000
#define NETWORK_IMG_SIZE 28 // pixel width and height of the images that the network can take

extern SDL_Renderer* renderer;
extern SDL_Window* window;

int InitSDL();
void DestroySDL();
