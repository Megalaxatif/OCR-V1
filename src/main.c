#include "header/math.h"
#include "header/init.h"
#include "header/image.h"
#include "header/neurons.h"

int main(){
    InitSDL();
    struct Mat* grayScale = GetGrayScaleMatrix("/home/megalaxatif/Documents/code/OCR/train/0/1.png");

    struct Layer* network[4] = {NULL};

    network[0] = CreateLayer(NETWORK_IMG_SIZE*NETWORK_IMG_SIZE, 16, NULL, NULL); // input
    network[1] = CreateLayer(16, 16, NULL, NULL);  // h1
    network[2] = CreateLayer(16, 10, NULL, NULL);  // h2
    network[3] = (struct Layer*){NULL, NULL, NULL}; // final layer (no weights nor biases)

    SDL_Event event;
    int running = 1;
    while (running){
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT)
                running = 0;
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderClear(renderer);

        DrawGrayScale(grayScale);

        SDL_RenderPresent(renderer);
        SDL_Delay(100); // delay to limit the frame rate
    }
    // cleaning
    for(int i = 0; i < 4; i++){
        DestroyLayer(network[i]);
    }
    DestroySDL();
    MatDestroy(grayScale);
    return 0;
}
