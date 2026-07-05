#include "header/math.h"
#include "header/neurons.h"
#include "header/init.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>


struct Mat* GetGrayScaleMatrix(char* imgFileName){ // loads the given image and returns a matrix of its grayscale
    SDL_Surface* surface = IMG_Load(imgFileName);
    if (surface == NULL){
        printf("Error: GetGrayScaleMatrix, impossible to load the image");
        return NULL;
    }

    int format = surface->format->format;
    if (format != SDL_PIXELFORMAT_INDEX8){
        printf("Error: GetGrayScaleMatrix, the image %s has the %s surface type but only SDL_PIXELFORMAT_INDEX8 is supported", imgFileName, SDL_GetPixelFormatName(format));
        SDL_FreeSurface(surface);
        return NULL;
    }

    size_t h = surface->h;
    size_t w = surface->w;
    Uint8* pixels = surface->pixels; // cast the void*
    SDL_Color* colorPalette = surface->format->palette->colors;
    struct Mat* grayScale = MatCreate(h, w, NULL);

    for(size_t y = 0; y < h; y++){
        for(size_t x = 0; x < w; x++){
            Uint8 colorId = *(pixels + y * surface->pitch + x);
            SDL_Color color = colorPalette[colorId];
            grayScale->data[y][x] =
                0.299 * color.r +
                0.587 * color.g +
                0.114 * color.b;
        }
    }
    SDL_FreeSurface(surface);
    return grayScale;
}

int DrawGrayScale(struct Mat* grayScale){ // display a grayScale matrix on the screen
    if (grayScale == NULL) {
        printf("Error: DisplayGrayScale, matrix pointer is NULL\n");
        return 1;
    }

    SDL_SetRenderTarget(renderer, NULL);

    size_t h = grayScale->row;
    size_t w = grayScale->col;
    for(size_t y = 0; y < h; y++){
        for(size_t x = 0; x < w; x++){
            Uint8 grayCode = grayScale->data[y][x];
            SDL_SetRenderDrawColor(renderer, grayCode, grayCode, grayCode, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    return 0;
}
