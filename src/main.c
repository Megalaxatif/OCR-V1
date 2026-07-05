#include "header/math.h"
#include "header/init.h"
#include "header/image.h"


int main(){
    InitSDL();
    struct Mat* grayScale = GetGrayScaleMatrix("/home/megalaxatif/Documents/code/OCR/cool dude.png");

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
    DestroySDL();
    MatDestroy(grayScale);
    return 0;
}
