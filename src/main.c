#include "header/init.h"
#include "header/ocr.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>


int main(){
    int errorCode = 0;
    time_t seed = 14554;
    srand(time(&seed)); // initialise the seed
    InitSDL();
    struct nk_context *ctx = InitNuklear();

    struct OCR* ocr = CreateOCR();

    int running = 1; // bool
    while (running){
        errorCode = Update(ctx, ocr);
        if (errorCode == -1){
            running = 0;
        };
    }
    cleanup:

    // clean the ocr structure
    DestroyOCR(ocr);

    // clean sdl
    DestroySDL();

    // clean nuklear
    nk_sdl_shutdown();

    printf("return code: %d\n", errorCode);
    return errorCode;
}
