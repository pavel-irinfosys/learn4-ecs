/**
 * main file
 */
#include <SDL3/SDL.h>
#include <cstdio>

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    std::printf("SDL3 initialized successfully.\n");
    std::printf("SDL3 compiled version: %d.%d.%d\n",
                SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);

    SDL_Quit();
    return 0;
}
