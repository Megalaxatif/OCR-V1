#include "header/init.h"
#include "header/ocr.h"

#include <time.h>


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
        SDL_Delay(1000/FPS);
    }
    // clean the ocr structure
    DestroyOCR(ocr);

    // clean sdl
    DestroySDL();

    // clean nuklear
    nk_sdl_shutdown();

    printf("return code: %d\n", errorCode);
    return errorCode;
}
