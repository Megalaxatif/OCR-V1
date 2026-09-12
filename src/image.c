#include "header/image.h"
#include "header/math.h"
#include "header/init.h"
#include "header/neurons.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <stdalign.h>

struct Mat* InvertForwardPassGrayScaleMatrix(struct Mat* grayScale){
    if (grayScale == NULL){
        printf("Error: InvertForwardPassGrayScaleMatrix, invalid argument");
        return NULL;
    }
    for(int i = 0; i < grayScale->row; i++){
        for(int j = 0; j < grayScale->col; j++){
            grayScale->data[i][j] = 1 - grayScale->data[i][j];
        }
    }
    return grayScale;
}

int DrawRects(SDL_Rect* rects, size_t rectCount, struct Mat* referenceMatrix, SDL_Color color){
    if (rects == NULL || rectCount <= 0 || referenceMatrix == NULL){
        printf("Error: DrawRect, invalid argument\n");
        return 1;
    }
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        referenceMatrix->col,
        referenceMatrix->row
    );
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    for(int i = 0; i < rectCount; i++ ){
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawRect(renderer, rects+i);
    }

    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_DestroyTexture(texture);
    return 0;
}

int DrawFilledRects(SDL_Rect* rects, size_t rectCount, struct Mat* referenceMatrix, SDL_Color color){
    if (rects == NULL || rectCount <= 0 || referenceMatrix == NULL){
        printf("Error: DrawRect, invalid argument\n");
        return 1;
    }
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        referenceMatrix->col,
        referenceMatrix->row
    );
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    for(int i = 0; i < rectCount; i++ ){
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, rects+i);
    }

    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_DestroyTexture(texture);
    return 0;
}

int DrawDigitGrayScales(struct Mat** digitGrayScales, SDL_Rect* rects, size_t rectCount, struct Mat* referenceMatrix){
    if (digitGrayScales == NULL || *digitGrayScales == NULL || rects == NULL || referenceMatrix == NULL){
        printf("Error: DrawDigitGrayScales, invalid argument\n");
        return 1;
    }
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        referenceMatrix->col,
        referenceMatrix->row
    );
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    for(int i = 0; i < rectCount; i++){
        struct Mat* currentGrayScale = digitGrayScales[i];
        //MatPrint(currentGrayScale);
        SDL_Rect currentRect = rects[i];
        if (currentGrayScale->col != 1 || currentGrayScale->row != NETWORK_IMG_SIZE*NETWORK_IMG_SIZE){
            printf("Error: DrawDigitGrayScales, the matrix given doesn't have valid dimensions for a digit grayscale");
            return 2;
        }
        for(size_t y = 0; y < NETWORK_IMG_SIZE; y++){
            for(size_t x = 0; x < NETWORK_IMG_SIZE; x++){
                Uint8 grayCode = currentGrayScale->data[y*NETWORK_IMG_SIZE + x][0]*255;
                SDL_SetRenderDrawColor(renderer, grayCode, grayCode, grayCode, 255);
                SDL_RenderDrawPoint(renderer, x + currentRect.x, y + currentRect.y);
            }
        }
    }
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_DestroyTexture(texture);
    return 0;
}


int DrawGrayScale(struct Mat* grayScale){
    if (grayScale == NULL) {
        printf("Error: DisplayGrayScale, matrix pointer is NULL\n");
        return 1;
    }
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        grayScale->col,
        grayScale->row
    );
    SDL_SetRenderTarget(renderer, texture);

    for(size_t y = 0; y < grayScale->row; y++){
        for(size_t x = 0; x < grayScale->col; x++){
            double grayCode = grayScale->data[y][x];
            SDL_SetRenderDrawColor(renderer, grayCode*255, grayCode*255, grayCode*255, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_DestroyTexture(texture);
    return 0;
}

// VERTICAL----------------------------------

SDL_Rect* ScanVerticalLines(struct Mat* grayScale, size_t* lineCount_){
    if (grayScale == NULL||lineCount_ == NULL){
        printf("Error: ScanVerticalLines, invalid argument\n");
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
                        if (currentLine.h > grayScale->row/10){ // ignore little lines
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
    if (grayScale == NULL || lineCount_ == NULL){
        printf("Error: ScanHorizontalLines, invalid argument\n");
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
                        if (currentLine.w > grayScale->col/10){ // ignore little lines
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

SDL_Rect* ConvertHorizontalLinesToBlocks(SDL_Rect* lines, size_t lineCount, size_t* blockCount_){
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
// -----------------------------------

int SortBlocks(SDL_Rect** horizontalBlocks, SDL_Rect** verticalBlocks, size_t* horizontalBlockCount, size_t* verticalBlockCount){
    if (horizontalBlocks == NULL || *horizontalBlocks == NULL || *verticalBlocks == NULL || verticalBlocks == NULL || horizontalBlockCount == NULL || verticalBlockCount == NULL){
        printf("Error: SortBlocks, invalid arguments\n");
        return 1;
    }
    SDL_Rect* sortedHorizontalBlocks = malloc(10*sizeof(SDL_Rect));
    SDL_Rect* sortedVerticalBlocks = malloc(10*sizeof(SDL_Rect));
    int sortedHorizontalBlockCount = 0;
    int verticalLinesFound = 0; // boolean

    for(size_t i = 0; i < *horizontalBlockCount; i++){
        int verticalCollisionCount = 0;
        for(size_t j = 0; j < *verticalBlockCount; j++){
            if (SDL_HasIntersection(*horizontalBlocks + i, *verticalBlocks + j)){
                if (!verticalLinesFound)
                    sortedVerticalBlocks[verticalCollisionCount] = (*verticalBlocks)[j];

                verticalCollisionCount++;
                if (verticalCollisionCount >= 10)
                    break;
            }
        }
        if (verticalCollisionCount == 10){
            verticalLinesFound = 1;
            sortedHorizontalBlocks[sortedHorizontalBlockCount] = (*horizontalBlocks)[i];
            sortedHorizontalBlockCount++;
            if (sortedHorizontalBlockCount == 10) break; // found all lines
        }
    }
    if (!verticalLinesFound || sortedHorizontalBlockCount != 10){
        printf("Warning: SortBlock, no valid grid found\n");
        free(sortedHorizontalBlocks);
        free(sortedVerticalBlocks);
        return 2;
    }
    free(*horizontalBlocks);
    free(*verticalBlocks);
    *horizontalBlocks = sortedHorizontalBlocks;
    *verticalBlocks = sortedVerticalBlocks;
    *horizontalBlockCount = 10;
    *verticalBlockCount = 10;
    return 0;
}

SDL_Rect* GetSudokuDigitRects(SDL_Rect* horizontalBlocks, SDL_Rect* verticalBlocks){
    if (horizontalBlocks == NULL || verticalBlocks == NULL){
        printf("Error: GetDigitRects, invalid argument\n");
        return NULL;
    }
    // a sudoku has 81 digits
    SDL_Rect* digitRects = malloc(81*sizeof(SDL_Rect));

    for(size_t i = 0; i < 9; i++){
        for(size_t j = 0; j < 9; j++){
            SDL_Rect res = {0};
            SDL_IntersectRect(horizontalBlocks + i, verticalBlocks + j, &res);
            SDL_Rect res2 = {0}; // diagonal to res
            SDL_IntersectRect(horizontalBlocks + i + 1, verticalBlocks + j + 1, &res2);
            int x = res.x + res.w + GRID_OFFSET;
            int y = res.y + res.h + GRID_OFFSET;
            int h = res2.y - y - GRID_OFFSET;
            int w = res2.x - x - GRID_OFFSET;
            digitRects[i*9+j] = (SDL_Rect){.x = x, .y = y, .h = h, .w = w};
        }
    }
    return digitRects;
}

struct Mat** ConvertTexturesToGrayScale(SDL_Texture** textures, size_t textureCount){
    if (textures == NULL || textureCount <= 0){
        printf("Error: ConvertTexturesToGrayScale, invalid argument\n");
        return NULL;
    }
    struct Mat** grayScales = malloc(textureCount * sizeof(struct Mat*));
    SDL_Surface* rgbaSurface = SDL_CreateRGBSurfaceWithFormat( 0, NETWORK_IMG_SIZE, NETWORK_IMG_SIZE, 32, SDL_PIXELFORMAT_RGBA8888);

    for(int i = 0; i < textureCount; i++){
        SDL_SetRenderTarget(renderer, textures[i]);
        SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA8888, rgbaSurface->pixels, rgbaSurface->pitch);

        grayScales[i] = GetForwardPassGrayScaleMatrix(rgbaSurface);
        //PrintDigitGrayScales(grayScales + i, 1);

        if (grayScales[i] == NULL){
            printf("Error: ConvertTexturesToGrayScale, GetForwardPassGrayScaleMatrix returned NULL\n");
            for(int j = 0; j < i; j++){
                MatDestroy(grayScales[j]);
            }
            free(grayScales);
            SDL_FreeSurface(rgbaSurface);
            return NULL;
        }
    }
    SDL_FreeSurface(rgbaSurface);
    return grayScales;
}

SDL_Texture** GetSudokuDigitTextures(SDL_Rect* digitRects, char* sudokuFilePath){
    if(digitRects == NULL || sudokuFilePath == NULL){
        printf("Error: GetSudokuDigits, invalid argument\n");
        return NULL;
    }

    SDL_Surface* sudoku = IMG_Load(sudokuFilePath); // TODO : give the sudoku as a texture in argument
    if (sudoku == NULL){
        printf("Error: GetSudokuDigits, impossible to load the image at %s\n", sudokuFilePath);
        return NULL;
    }

    SDL_Texture* sudokuTexture = SDL_CreateTextureFromSurface(renderer, sudoku);
    SDL_FreeSurface(sudoku);

    SDL_Texture** digitTextures = malloc(81*sizeof(SDL_Texture*));
    for(int i = 0; i < 81; i++){
        digitTextures[i] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, NETWORK_IMG_SIZE, NETWORK_IMG_SIZE);
    }

    for(int i = 0; i < 9; i++){
        for(int j = 0; j < 9; j++){
            SDL_SetRenderTarget(renderer, digitTextures[i*9+j]);
            SDL_RenderCopy( renderer, sudokuTexture, digitRects + i*9+j, NULL);
        }
    }

    SDL_SetRenderTarget(renderer, NULL);
    SDL_DestroyTexture(sudokuTexture);
    return digitTextures;
}

struct Mat* GetGridGrayScaleMatrix(char* imgFileName){
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
                double grayCode = // TODO: use GetRGBA here
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

struct Mat* GetForwardPassGrayScaleMatrix(SDL_Surface* surface){
    if (surface == NULL){
        printf("Error: GetForwardPassGrayScaleMatrix, surface is NULL\n");
        return NULL;
    }
    if(surface->h != NETWORK_IMG_SIZE|| surface->w != NETWORK_IMG_SIZE){
        printf("Error: GetForwardPassGrayScaleMatrix, the width and height of the surface doesn't match the value of the NETWORK_IMG_SIZE constant which is set to %d pixels\n", NETWORK_IMG_SIZE);
        return NULL;
    }

    int format = surface->format->format;
    if (format != SDL_PIXELFORMAT_RGBA8888){
        printf("Error: GetForwardPassGrayScaleMatrix, the surface given has the %s surface type but only SDL_PIXELFORMAT_RGBA8888 is supported\n", SDL_GetPixelFormatName(format));
        return NULL;
    }

    Uint8* pixels = surface->pixels; // cast the void*
    struct Mat* grayScale = MatCreate(NETWORK_IMG_SIZE*NETWORK_IMG_SIZE, 1, NULL, NULL); // the matrix must be a column matrix for the network

    for(size_t y = 0; y < NETWORK_IMG_SIZE; y++){
        for(size_t x = 0; x < NETWORK_IMG_SIZE; x++){
            Uint32 rawPixel = *(Uint32 *)(pixels + y * surface->pitch + x * 4);
            Uint8 r, g, b, a;
            SDL_GetRGBA(rawPixel, surface->format, &r, &g, &b, &a);
            //SDL_Color color = colorPalette[colorId];
            double grayCode = 0.299 * r/255 + 0.587 * g/255 + 0.114 * b/255;
            grayScale->data[y*NETWORK_IMG_SIZE + x][0] = grayCode;
            //printf("%.1f ", grayCode);
            //printf("(%d,%d,%d)", r, g, b);
        }
        //printf("\n");
    }
    return grayScale;
}
