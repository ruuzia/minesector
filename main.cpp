#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <cstdio>

#include "common.h"
#include "texture.h"
#include "button.h"
#include "game.h"

const int SCREEN_WIDTH =  640 * 1.5;
const int SCREEN_HEIGHT = 480 * 1.5;

static bool init(SDL_Renderer * &renderer, SDL_Window * &window) {
    window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Unable to create window. SDL Error: %s\n", SDL_GetError());
        return FAIL;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Unable to create accelerated renderer. SDL Error: %s\n", SDL_GetError());
        return FAIL;
    }

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags)& imgFlags)) {
        printf("SDL_image could not initialize. SDL_image Error: %s\n", IMG_GetError());
        return FAIL;
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize. SDL_ttf error: %s\n", TTF_GetError());
    }
    
    return OK;
}


static bool loadMedia(SDL_Window *window, Game &game) {
    if (game.loadMedia(window) == FAIL) {
        return FAIL;
    }
    return OK;
}

[[ noreturn ]] static void close(int exitCode, SDL_Renderer *renderer, SDL_Window *window) {
    SDL_DestroyRenderer(renderer);
    renderer = NULL;

    SDL_DestroyWindow(window);
    window = NULL;

    IMG_Quit();
    SDL_Quit();

    exit(exitCode);
}

static bool Update(SDL_Renderer *renderer, Game &game, double dt) {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            return true;

        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_t) {
                printf("dt: %f\n", dt);
            }
            break;
        
        case SDL_MOUSEBUTTONDOWN:
            game.onMouseButtonDown(e.button);
            break;

        case SDL_MOUSEBUTTONUP:
            game.onMouseButtonUp(e.button);
            break;

        case SDL_MOUSEMOTION:
            game.onMouseMove(&e, e.motion.x, e.motion.y);
            break;
        }
    }

    game.OnUpdate(dt);
    SDL_RenderPresent(renderer);
    
    return false;
}

const int TITLE_FONT_SIZE = 28;

int main(int argc, char **argv) {
    (void)argc; (void) argv;

    Game game;
    SDL_Renderer *renderer;
    SDL_Window *window;

    if (init(renderer, window) == FAIL) {
        close(1, renderer, window);
    }

    if (loadMedia(window, game) == FAIL) {
        close(1, renderer, window);
    }

    bool quit = false;
    Uint32 lastFrame = SDL_GetTicks();
    game.OnStart();
    while (!quit) {
        Uint32 current = SDL_GetTicks();
        double dt = (current - lastFrame) / 1000;
        lastFrame = current;
        quit = Update(renderer, game, dt);
    }

    close(0, renderer, window);
}
