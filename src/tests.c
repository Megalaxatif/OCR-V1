#include "header/neurons.h"

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
