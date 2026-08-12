#include "header/math.h"
#include "header/init.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <stdalign.h>

int DrawHorizontalLines(SDL_Rect* horizontalLines, size_t lineCount){
    if (horizontalLines == NULL || lineCount < 2){
        printf("Error: DrawHorizontalLines, invalid argument\n");
        return 1;
    }
    SDL_SetRenderTarget(renderer, NULL);
    int colorOffset = 255;
    for(int i = 0; i < lineCount; i++ ){
        SDL_SetRenderDrawColor(renderer, colorOffset, 0,0, 128);
        SDL_RenderDrawRect(renderer, horizontalLines+i);
        //colorOffset = colorOffset - 20;
        //if (colorOffset <100) colorOffset = 255;
    }
    return 0;
}

SDL_Rect* ScanHorizontalLines(struct Mat* grayScale, size_t* lineCount_){
    if (grayScale == NULL){
        printf("Error: ScanHorizontalLines, grayScale is NULL\n");
        return NULL;
    }
    // TODO: remove that and use a point buffer instead (use one similar to minimake)
    const int lineArraySize = 2000;
    SDL_Rect* lines = malloc(lineArraySize* sizeof(SDL_Rect));

    size_t lineCount = 0;
    for(size_t y = 0; y < grayScale->row; y++){
        SDL_Rect currentLine = {.x = 0, .y = y, .w = 0, .h = 0}; // first point of the line
        int inLine = 0; // boolean
        for(size_t x = 0; x < grayScale->col; x++){
            int grayCode = grayScale->data[y][x];
            if (inLine){
                if (grayCode == 0)
                    currentLine.w++;
                else {
                    int isHole = 0;
                    int i = 1;
                    while(x+i < grayScale->col && i < grayScale->col/30){
                        if (grayScale->data[y][x+i] == 0){
                            x += i;
                            currentLine.w += i;
                            isHole = 1;
                            break;
                        }
                        i++;
                    }
                    if (!isHole) { // end of the line
                        if (currentLine.w > grayScale->col/4){ // ignore little lines
                            // TODO: remove that and use a point buffer instead (use one similar to minimake)
                            lines[lineCount] = currentLine;
                            lineCount++;
                            if (lineCount >= lineArraySize){
                                printf("Error: ScanHorizontalLines, the line array is full\n");
                                free(lines);
                                return NULL;
                            }
                        }
                        inLine = 0;
                        currentLine.w = 0;
                    }
                }
            }
            else {
                if (grayCode == 0){ // we are going inside a potential line
                    currentLine.w++;
                    currentLine.x = x;
                    inLine = 1;
                }
            }
        }
    }
    *lineCount_ = lineCount;
    return lines;
}

// SDL_Rect* ConvertHorizontalLinesToRect(SDL_Point* points, size_t pointCount, size_t* rectCount_){ // convert adjacent horizontal lines into a rectangle
//     if (points == NULL || rectCount_ == NULL || pointCount < 0||pointCount %2 != 0){
//         printf("Error: ConvertHorizontalLinesToRect, invalid argument");
//         return NULL;
//     }
//     SDL_Rect* rectList = malloc(200*sizeof(struct SDL_Rect)); // TODO: change this constant to use buffer just like in ScanHorizontalLines
//     int rectCount = 0;
//     int lastPoint = points[0].y;
//     int lineCount = 1; // count the number of lines to merge into the rectangle
//     for (size_t i = 1; i < pointCount/2; i++){
//         SDL_Point currentPoint = points[i*2];
//         if (currentPoint.y == lastY+1 && ){
//             lineCount++;
//         }
//         else{
//             if (lineCount != 0){
//                 rectList[rectCount] = (SDL_Rect){};
//                 lineCount = 0;
//             }
//         }
//         lastY = points[i].y;
//     }
//     return rectList;
// }

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
