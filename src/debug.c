#include "header/neurons.h"
#include "header/image.h"

void PrintDigitGrayScales(struct Mat** digitGrayScale, size_t grayScaleCount){
    if (digitGrayScale == NULL){
        printf("Error: MatPrint, matrix pointer is NULL\n");
        return;
    }
    for(size_t i = 0; i < grayScaleCount; i++){
        struct Mat* currentGrayScale = digitGrayScale[i];
        if (currentGrayScale->col != 1 || currentGrayScale->row != NETWORK_IMG_SIZE*NETWORK_IMG_SIZE){
            printf("Error: PrintDigitGrayScale, the matrix given doesn't have valid dimensions for a digit grayscale");
            return;
        }
        for(size_t i = 0; i < NETWORK_IMG_SIZE; i++){
            for(size_t j = 0; j < NETWORK_IMG_SIZE; j++){
                printf("%.1f ", currentGrayScale->data[i*NETWORK_IMG_SIZE+j][0]);
            }
            printf("\n");
        }
        printf("\n");
    }
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

    for(size_t i = 0; i < rectCount; i++){
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
