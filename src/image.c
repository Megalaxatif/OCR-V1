#include "header/math.h"
#include "header/init.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <stdalign.h>

int DrawRect(SDL_Rect* rects, size_t rectCount){
    if (rects == NULL || rectCount <= 0){
        printf("Error: DrawRect, invalid argument\n");
        return 1;
    }
    SDL_SetRenderTarget(renderer, NULL);
    int colorOffset = 255;
    for(int i = 0; i < rectCount; i++ ){
        SDL_SetRenderDrawColor(renderer, colorOffset, 0,0, 255);
        SDL_RenderDrawRect(renderer, rects+i);
        //colorOffset = colorOffset - 20;
        //if (colorOffset <100) colorOffset = 255;
    }
    return 0;
}

// VERTICAL----------------------------------

SDL_Rect* ScanVerticalLines(struct Mat* grayScale, size_t* lineCount_){
    if (grayScale == NULL){
        printf("Error: ScanVerticalLines, grayScale is NULL\n");
        return NULL;
    }
    // TODO: remove that and use a point buffer instead (use one similar to minimake)
    const int lineArraySize = 2000;
    SDL_Rect* lines = malloc(lineArraySize* sizeof(SDL_Rect));

    size_t lineCount = 0;
    SDL_Rect currentLine = {.x = 0, .y = 0, .w = 0, .h = 1}; // first point of the line
    for(size_t x = 0; x < grayScale->col; x++){
        currentLine.x = x;
        currentLine.y = 0;
        currentLine.w = 0;
        int inLine = 0; // boolean
        for(size_t y = 0; y < grayScale->row; y++){
            int grayCode = grayScale->data[y][x];
            if (inLine){
                if (grayCode == 0)
                    currentLine.h++;
                else {
                    int isHole = 0;
                    int i = 1;
                    while(y+i < grayScale->row && i < grayScale->row/100){  // NOTE: adjust this value if necessary
                        if (grayScale->data[y+i][x] == 0){
                            y += i;
                            currentLine.h += i;
                            isHole = 1;
                            break;
                        }
                        i++;
                    }
                    if (!isHole) { // end of the line
                        if (currentLine.h > grayScale->row/5){ // ignore little lines
                            // TODO: remove that and use a point buffer instead (use one similar to minimake)
                            lines[lineCount] = currentLine;
                            lineCount++;
                            if (lineCount >= lineArraySize){
                                printf("Error: ScanVerticalLines, the line array is full\n");
                                free(lines);
                                *lineCount_ = 0;
                                return NULL;
                            }
                        }
                        inLine = 0;
                    }
                }
            }
            else {
                if (grayCode == 0){ // we are going inside a potential line
                    currentLine.h = 1;
                    currentLine.y = y;
                    inLine = 1;
                }
            }
        }
    }
    *lineCount_ = lineCount;
    return lines;
}

SDL_Rect* ConvertVerticalLinesToBlocks(SDL_Rect* lines, size_t lineCount, size_t* blockCount_){ // convert adjacent vertical lines into a rectangle
    if (lines == NULL || blockCount_ == NULL|| lineCount <= 0){
        printf("Error: ConvertVerticalLinesToBlocks, invalid argument\n");
        return NULL;
    }
    SDL_Rect* blockList = malloc(200*sizeof(SDL_Rect)); // TODO: change this constant to use buffer just like in ScanVerticalLines
    size_t blockCount = 0;
    size_t lineConvertedCount = 0;
    int blockMinY = 0;
    int blockMaxH = 0;
    int blockX = 0;
    int blockWidth = 0;
    SDL_Rect prevLine = lines[0];
    SDL_Rect adjacentLine = (SDL_Rect){.x = 0, .y = 0, .w = 0, .h = 0};
    size_t i = 0;
    while(lineConvertedCount != lineCount){
        if (blockWidth == 0){ // we are creating a new block
            adjacentLine = lines[i];
            if (adjacentLine.x == 0 && adjacentLine.y == 0 && adjacentLine.w == 0 && adjacentLine.h == 0){
                i++;
                continue; // ignore the lines already converted
            }
            blockX = adjacentLine.x;
            blockMinY = adjacentLine.y;
            blockMaxH = adjacentLine.h;
            lines[i] = (SDL_Rect){.x = 0, .y = 0, .w = 0, .h = 0}; // remove it from the list to avoid repetition
            lineConvertedCount++;
            blockWidth ++;
        }
        else{
            int delta = 0.05*prevLine.h;
            int adjacentLineFound = 0;
            int j = i+1; // all the lines before i are above so no need to check them
            while(!adjacentLineFound && j < lineCount){
                adjacentLine = lines[j];
                if (adjacentLine.x == 0 && adjacentLine.y == 0 && adjacentLine.w == 0 && adjacentLine.h == 0) {
                    j++;
                    continue; // ignore the lines already converted
                }
                if (adjacentLine.x == prevLine.x + 1 &&
                    abs(adjacentLine.y - prevLine.y) < delta &&
                    abs(adjacentLine.h - prevLine.h) < delta)
                {
                    if (adjacentLine.y < blockMinY)
                        blockMinY = adjacentLine.y;
                    if (adjacentLine.h > blockMaxH)
                        blockMaxH = adjacentLine.h;

                    adjacentLineFound = 1;
                    lines[j] = (SDL_Rect){.x = 0, .y = 0, .w = 0, .h = 0}; // remove it from the list to avoid repetition
                    lineConvertedCount++;
                    blockWidth++;
                }
                j++;
            }

            if (!adjacentLineFound){
                blockList[blockCount] = (SDL_Rect){.x = blockX, .y = blockMinY, .w = blockWidth, .h = blockMaxH};
                blockCount++;
                i++;
                blockMinY = 0;
                blockMaxH = 0;
                blockWidth = 0;
            }
        }
        prevLine = adjacentLine;
    }
    // add the last block
    if (blockWidth != 0){
        blockList[blockCount] = (SDL_Rect){.x = blockX, .y = blockMinY, .w = blockWidth, .h = blockMaxH};
        blockCount++;
    }
    printf("block count: %ld\n", blockCount);
    *blockCount_ = blockCount;
    return blockList;
}

// HORIZONTAL----------------------------------

SDL_Rect* ScanHorizontalLines(struct Mat* grayScale, size_t* lineCount_){
    if (grayScale == NULL){
        printf("Error: ScanHorizontalLines, grayScale is NULL\n");
        return NULL;
    }
    // TODO: remove that and use a point buffer instead (use one similar to minimake)
    const int lineArraySize = 2000;
    SDL_Rect* lines = malloc(lineArraySize* sizeof(SDL_Rect));

    size_t lineCount = 0;
    SDL_Rect currentLine = {.x = 0, .y = 0, .w = 0, .h = 1}; // first point of the line
    for(size_t y = 0; y < grayScale->row; y++){
        currentLine.x = 0;
        currentLine.y = y;
        currentLine.w = 0;
        int inLine = 0; // boolean
        for(size_t x = 0; x < grayScale->col; x++){
            int grayCode = grayScale->data[y][x];
            if (inLine){
                if (grayCode == 0)
                    currentLine.w++;
                else {
                    int isHole = 0;
                    int i = 1;
                    while(x+i < grayScale->col && i < grayScale->col/100){  // NOTE: adjust this value if necessary
                        if (grayScale->data[y][x+i] == 0){
                            x += i;
                            currentLine.w += i;
                            isHole = 1;
                            break;
                        }
                        i++;
                    }
                    if (!isHole) { // end of the line
                        if (currentLine.w > grayScale->col/5){ // ignore little lines
                            // TODO: remove that and use a point buffer instead (use one similar to minimake)
                            lines[lineCount] = currentLine;
                            lineCount++;
                            if (lineCount >= lineArraySize){
                                printf("Error: ScanHorizontalLines, the line array is full\n");
                                free(lines);
                                *lineCount_ = 0;
                                return NULL;
                            }
                        }
                        inLine = 0;
                    }
                }
            }
            else {
                if (grayCode == 0){ // we are going inside a potential line
                    currentLine.w = 1;
                    currentLine.x = x;
                    inLine = 1;
                }
            }
        }
    }
    *lineCount_ = lineCount;
    return lines;
}

SDL_Rect* ConvertHorizontalLinesToBlocks(SDL_Rect* lines, size_t lineCount, size_t* blockCount_){ // convert adjacent horizontal lines into a rectangle
    if (lines == NULL || blockCount_ == NULL|| lineCount <= 0){
        printf("Error: ConvertHorizontalLinesToBlocks, invalid argument\n");
        return NULL;
    }
    SDL_Rect* blockList = malloc(200*sizeof(SDL_Rect)); // TODO: change this constant to use buffer just like in ScanHorizontalLines
    size_t blockCount = 0;
    size_t lineConvertedCount = 0;
    int blockMinX = 0;
    int blockMaxW = 0;
    int blockY = 0;
    int blockHeight = 0;
    SDL_Rect prevLine = lines[0];
    SDL_Rect adjacentLine = (SDL_Rect){.x = 0, .y = 0, .w = 0, .h = 0};
    size_t i = 0;
    while(lineConvertedCount != lineCount){
        if (blockHeight == 0){ // we are creating a new block
            adjacentLine = lines[i];
            if (adjacentLine.x == 0 && adjacentLine.y == 0 && adjacentLine.w == 0 && adjacentLine.h == 0){
                i++;
                continue; // ignore the lines already converted
            }
            blockY = adjacentLine.y;
            blockMinX = adjacentLine.x;
            blockMaxW = adjacentLine.w;
            lines[i] = (SDL_Rect){.x = 0, .y = 0, .w = 0, .h = 0}; // remove it from the list to avoid repetition
            lineConvertedCount++;
            blockHeight ++;
        }
        else{
            int delta = 0.05*prevLine.w;
            int adjacentLineFound = 0;
            int j = i+1; // all the lines before i are above so no need to check them
            while(!adjacentLineFound && j < lineCount){
                adjacentLine = lines[j];
                if (adjacentLine.x == 0 && adjacentLine.y == 0 && adjacentLine.w == 0 && adjacentLine.h == 0) {
                    j++;
                    continue; // ignore the lines already converted
                }
                if (adjacentLine.y == prevLine.y + 1 &&
                    abs(adjacentLine.x - prevLine.x) < delta &&
                    abs(adjacentLine.w - prevLine.w) < delta)
                {
                    if (adjacentLine.x < blockMinX)
                        blockMinX = adjacentLine.x;
                    if (adjacentLine.w > blockMaxW)
                        blockMaxW = adjacentLine.w;

                    adjacentLineFound = 1;
                    lines[j] = (SDL_Rect){.x = 0, .y = 0, .w = 0, .h = 0}; // remove it from the list to avoid repetition
                    lineConvertedCount++;
                    blockHeight++;
                }
                j++;
            }

            if (!adjacentLineFound){
                blockList[blockCount] = (SDL_Rect){.x = blockMinX, .y = blockY, .w = blockMaxW, .h = blockHeight};
                blockCount++;
                i++;
                blockMinX = 0;
                blockMaxW = 0;
                blockHeight = 0;
            }
        }
        prevLine = adjacentLine;
    }
    // add the last block
    if (blockHeight != 0){
        blockList[blockCount] = (SDL_Rect){.x = blockMinX, .y = blockY, .w = blockMaxW, .h = blockHeight};
        blockCount++;
    }

    *blockCount_ = blockCount;
    return blockList;
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
