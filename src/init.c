#include "header/init.h"
#include <stdio.h>
// int InitImGui(){
//     // TODO: error handling
//     IMGUI_CHECKVERSION();
//     ImGui::CreateContext();
//     ImGuiIO& io = ImGui::GetIO(); (void)io;
//     ImGui::StyleColorsDark();
//     ImGui_ImplSDL2_InitForSDLRenderer(window, renderer); // initialise ImGui
//     ImGui_ImplSDLRenderer2_Init(renderer);
//     return 0;
// }

// init of the two global variables
SDL_Renderer* renderer = NULL;
SDL_Window* window = NULL;


int InitSDL(){
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL init error" );
        return -1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        SDL_Log("IMG_Init error: %s", IMG_GetError());
        return -1;
    }

    if (TTF_Init() != 0) {
        SDL_Log("Erreur SDL_ttf : %s\n", TTF_GetError());
        return -1;
    }

    window = SDL_CreateWindow(
        "OCR",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_W,
        SCREEN_H,
        SDL_WINDOW_SHOWN
    );

    if(!window){
        printf("Window initialisation error");
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if(!renderer){
        printf("renderer initialisation error");
        return -1;
    }

    // log
    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    printf("Renderer name: %s\n", info.name);
    printf("Version compile-time : %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);

    return 0;
}

// void DestroyImGui(){
//     ImGui_ImplSDLRenderer2_Shutdown();
//     ImGui_ImplSDL2_Shutdown();
//     ImGui::DestroyContext();
// }

void DestroySDL(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}
