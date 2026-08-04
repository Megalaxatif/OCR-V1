#include "header/math.h"
#include "header/init.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <stdalign.h>

int DrawHorizontalLines(SDL_Point* horizontalLines, size_t pointCount){
    if (horizontalLines == NULL || pointCount < 2){
        printf("Error: DrawHorizontalLines, invalid argument\n");
        return 1;
    }
    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
    for(int i = 0; i < pointCount; i += 2 ){
        SDL_Point firstPoint = horizontalLines[i];
        SDL_Point secondPoint = horizontalLines[i+1];
        SDL_RenderDrawLine(renderer, firstPoint.x, firstPoint.y, secondPoint.x, secondPoint.y);
    }
    return 0;
}

SDL_Point* ScanHorizontalLines(struct Mat* grayScale, size_t* pointCount_){
    if (grayScale == NULL){
        printf("Error: ScanHorizontalLines, grayScale is NULL\n");
        return NULL;
    }
    // TODO: remove that and use a point buffer instead (use one similar to minimake)
    const int pointArraySize = 2000;
    SDL_Point* points = malloc(pointArraySize* sizeof(SDL_Point));

    SDL_Point firstPoint = {.x = 0, .y = 0}; // first point of the line
    size_t pointCount = 0;
    int inLine = 0; // boolean
    size_t currentLineLength = 0;
    for(size_t y = 0; y < grayScale->row; y++){
        for(size_t x = 0; x < grayScale->col; x++){
            int grayCode = grayScale->data[y][x];
            if (inLine){
                if (grayCode == 0)
                    currentLineLength++;
                else {
                    int isHole = 0;
                    int i = 1;
                    while(x+i < grayScale->col && i < 20){
                        if (grayScale->data[y][x+i] == 0){
                            x += i;
                            isHole = 1;
                            break;
                        }
                        i++;
                    }
                    if (!isHole) { // end of the line
                        if (currentLineLength > 100){ // ignore little lines
                            // TODO: remove that and use a point buffer instead (use one similar to minimake)
                            if (pointCount + 2 >= pointArraySize){
                                printf("Error: ScanHorizontalLines, the point array is full\n");
                                return points;
                            }
                            points[pointCount] = firstPoint;
                            points[pointCount+1] = (SDL_Point){.x = x, .y = y};
                            pointCount+=2;
                        }
                        currentLineLength = 0;
                        inLine = 0;
                    }
                }
            }
            else {
                if (grayCode == 0){ // we are going inside a potential line
                    firstPoint.x = x;
                    firstPoint.y = y;
                    currentLineLength++;
                    inLine = 1;
                }
            }
        }
        currentLineLength = 0;
        inLine = 0;
    }
    *pointCount_ = pointCount;
    return points;
}


struct Mat* GetGridGrayScaleMatrix(char* imgFileName){ // loads the given image and returns a matrix of its grayscale
    if (imgFileName == NULL){
        printf("Error: GetGridGrayScaleMatrix, imgFileName is NULL\n");
        return NULL;
    }

    SDL_Surface* surface = IMG_Load(imgFileName);

    if (surface == NULL){
        printf("Error: GetGridGrayScaleMatrix, impossible to load the image at %s\n", imgFileName);
        return NULL;
    }

    int format = surface->format->format;
    if (format != SDL_PIXELFORMAT_INDEX8 && format != SDL_PIXELFORMAT_RGB24){
        printf("Error: GetGridGrayScaleMatrix, the image %s has the %s surface type but only SDL_PIXELFORMAT_INDEX8 and SDL_PIXELFORMAT_RGB24 are supported\n", imgFileName, SDL_GetPixelFormatName(format));
        SDL_FreeSurface(surface);
        return NULL;
    }

    Uint8* pixels = surface->pixels; // cast the void*
    struct Mat* grayScale = MatCreate(surface->h, surface->w, NULL, NULL);

    if (format == SDL_PIXELFORMAT_INDEX8){
        SDL_Color* colorPalette = surface->format->palette->colors;

        for(size_t y = 0; y < surface->h; y++){
            for(size_t x = 0; x < surface->w; x++){
                Uint8 colorId = *(pixels + y * surface->pitch + x);
                SDL_Color color = colorPalette[colorId];
                double grayCode =
                    0.299 * color.r/255 +
                    0.587 * color.g/255 +
                    0.114 * color.b/255;
                grayScale->data[y][x] = grayCode < 0.8 ? 0 : 1;

            }
        }
    }
    else if (format == SDL_PIXELFORMAT_RGB24){
        for(size_t y = 0; y < surface->h; y++){
            for(size_t x = 0; x < surface->w; x++){
                Uint8* pixel = pixels + y * surface->pitch + x*3;
                double grayCode =
                    0.299 * pixel[0]/255 +
                    0.587 * pixel[1]/255 +
                    0.114 * pixel[2]/255;
                grayScale->data[y][x] = grayCode < 0.8 ? 0 : 1;
            }
        }
    }
    SDL_FreeSurface(surface);
    return grayScale;
}

struct Mat* GetTrainingGrayScaleMatrix(char imgFileName[]){
    SDL_Surface* surface = IMG_Load(imgFileName);

    if (surface == NULL){
        printf("Error: GetTrainingGrayScaleMatrix, impossible to load the image at %s\n", imgFileName);
        return NULL;
    }

    if(surface->h != NETWORK_IMG_SIZE|| surface->w != NETWORK_IMG_SIZE){
        printf("Error: GetTrainingGrayScaleMatrix, the width and height of the image %s doesn't match the value of the NETWORK_IMG_SIZE constant which is set to %d pixels\n", imgFileName, NETWORK_IMG_SIZE);
        SDL_FreeSurface(surface);
        return NULL;
    }


    int format = surface->format->format;
    if (format != SDL_PIXELFORMAT_INDEX8){
        printf("Error: GetTrainingGrayScaleMatrix, the image %s has the %s surface type but only SDL_PIXELFORMAT_INDEX8 is supported\n", imgFileName, SDL_GetPixelFormatName(format));
        SDL_FreeSurface(surface);
        return NULL;
    }

    Uint8* pixels = surface->pixels; // cast the void*
    SDL_Color* colorPalette = surface->format->palette->colors;
    struct Mat* grayScale = MatCreate(NETWORK_IMG_SIZE*NETWORK_IMG_SIZE, 1, NULL, NULL); // the matrix must be a column matrix for the network

    for(size_t y = 0; y < NETWORK_IMG_SIZE; y++){
        for(size_t x = 0; x < NETWORK_IMG_SIZE; x++){
            Uint8 colorId = *(pixels + y * surface->pitch + x);
            SDL_Color color = colorPalette[colorId];
            grayScale->data[y*NETWORK_IMG_SIZE + x][0] =
                0.299 * color.r/255 +
                0.587 * color.g/255 +
                0.114 * color.b/255;
        }
    }
    SDL_FreeSurface(surface);
    return grayScale;
}

int DrawGrayScale(struct Mat* grayScale){
    if (grayScale == NULL) {
        printf("Error: DisplayGrayScale, matrix pointer is NULL\n");
        return 1;
    }
    SDL_SetRenderTarget(renderer, NULL);

    for(size_t y = 0; y < grayScale->row; y++){
        for(size_t x = 0; x < grayScale->col; x++){
            double grayCode = grayScale->data[y][x];
            SDL_SetRenderDrawColor(renderer, grayCode*255, grayCode*255, grayCode*255, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    return 0;
}
