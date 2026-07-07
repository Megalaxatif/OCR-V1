#include "header/math.h"
#include "header/init.h"
#include "header/image.h"


int main(){
    double r1[] = {-1, 2, 3};
    double r2[] = {-0, -2, 1};
    double r3[] = {-3, 1, 2};

    double* v1[] =
        {
            r1,
            r2,
            r3
        };

    struct Mat* mat_1 = MatCreate(3, 3,  v1);
    printf("mat1:\n");
    MatPrint(mat_1);
    printf("\n");
    mat_1 = MatFunc(mat_1, Relu);
    MatPrint(mat_1);

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
    MatDestroy(mat_1);
    return 0;
}
