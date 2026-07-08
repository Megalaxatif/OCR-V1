#include "header/math.h"
#include "header/neurons.h"
#include "header/init.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>


struct Mat* GetGrayScaleMatrix(char* imgFileName){ // loads the given image and returns a matrix of its grayscale
    SDL_Surface* surface = IMG_Load(imgFileName);

    if (surface == NULL){
        printf("Error: GetGrayScaleMatrix, impossible to load the image\n");
        return NULL;
    }

    if(surface->h != NETWORK_IMG_SIZE|| surface->w != NETWORK_IMG_SIZE){
        printf("Error: GetGrayScaleMatrix, the width and height of the image %s doesn't match the value of the NETWORK_IMG_SIZE constant which is set to %d pixels\n", imgFileName, NETWORK_IMG_SIZE);
        SDL_FreeSurface(surface);
        return NULL;
    }


    int format = surface->format->format;
    if (format != SDL_PIXELFORMAT_INDEX8){
        printf("Error: GetGrayScaleMatrix, the image %s has the %s surface type but only SDL_PIXELFORMAT_INDEX8 is supported\n", imgFileName, SDL_GetPixelFormatName(format));
        SDL_FreeSurface(surface);
        return NULL;
    }

    Uint8* pixels = surface->pixels; // cast the void*
    SDL_Color* colorPalette = surface->format->palette->colors;
    struct Mat* grayScale = MatCreate(NETWORK_IMG_SIZE*NETWORK_IMG_SIZE, 1, NULL); // the matrix must be a column matrix for the network

    for(size_t y = 0; y < NETWORK_IMG_SIZE; y++){
        for(size_t x = 0; x < NETWORK_IMG_SIZE; x++){
            Uint8 colorId = *(pixels + y * surface->pitch + x);
            SDL_Color color = colorPalette[colorId];
            grayScale->data[y*NETWORK_IMG_SIZE + x][0] =
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
    if (grayScale->col > 1){
        printf("Error: DisplayGrayScale, the matrix given must be a column matrix but this one has %ld columns, use GetGrayScaleMatrix to get a valid matrix\n", grayScale->col);
        return 2;
    }
    SDL_SetRenderTarget(renderer, NULL);

    for(size_t y = 0; y < NETWORK_IMG_SIZE; y++){
        for(size_t x = 0; x < NETWORK_IMG_SIZE; x++){
            Uint8 grayCode = grayScale->data[y*NETWORK_IMG_SIZE + x][0];
            SDL_SetRenderDrawColor(renderer, grayCode, grayCode, grayCode, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    return 0;
}
