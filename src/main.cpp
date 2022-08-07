#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <cstdio>

#include "texture.h"
#include "game.h"

const int SCREEN_WIDTH =  640 * 1.5;
const int SCREEN_HEIGHT = 480 * 1.5;

SDL_Renderer *renderer;

class SDL {
public:
    SDL() {
        window = nullptr;
        renderer = nullptr;
    }
    ~SDL() {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;

        SDL_DestroyWindow(window);
        window = nullptr;
    }

    void init();
    SDL_Window *window;
};

void SDL::init() {
    window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) throw std::runtime_error("Unable to create window. SDL Error: " + std::string(SDL_GetError()));

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
        throw std::runtime_error("Unable to create accelerated renderer. SDL Error: " + std::string(SDL_GetError()));

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
        throw std::runtime_error("SDL_image could not initialize. SDL_image Error: " + std::string(IMG_GetError()));

    if (TTF_Init() == -1) {
        throw std::runtime_error("SDL_ttf could not initialize. SDL_ttf error: " + std::string(TTF_GetError()));
    }
}


static void loadMedia(SDL const& sdl, Game &game) {
    game.loadMedia(sdl.window);
}

static bool Update(Game &game, double dt) {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
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
            game.onMouseMove(e.motion);
            break;
        }
    }

    game.OnUpdate(dt);
    SDL_RenderPresent(renderer);
    
    return false;
}

int main(int argc, char **argv) {
    (void)argc; (void) argv;

    SDL sdl;

    Game game(14, 14);


    try {
        sdl.init();
    }
    catch (const std::runtime_error& ex) {
        printf("Failed to initialize SDL.\n%s\n", ex.what());
        return 1;
    }

    try {
        loadMedia(sdl, game);
    }
    catch (const std::runtime_error& ex) {
        printf("Failed to load media.\n%s\n", ex.what());
        return 1;
    }

    bool quit = false;
    Uint32 lastFrame = SDL_GetTicks();
    game.OnStart();

    while (!quit) {
        Uint32 current = SDL_GetTicks();
        double dt = (current - lastFrame) / 1000.0;
        lastFrame = current;
        quit = Update(game, dt);
    }

    return 0;
}
