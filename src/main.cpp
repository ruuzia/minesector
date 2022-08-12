// I need this on Windows for some reason
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

        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
    }

    void init();
    SDL_Window *window;
};

void SDL::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL could not initialize! SDL Error: " + std::string(SDL_GetError()));
    }

    window = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
}


static bool Update(Game &game, double dt) {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderClear(renderer);

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            return false;

        case SDL_KEYDOWN:
            // Force quit with Alt + F4
            if (e.key.keysym.sym == SDLK_F4 && (e.key.keysym.mod & KMOD_ALT)) {
                printf("Force quit.\n");
                return false;
            }
            else if (e.key.keysym.sym == SDLK_q && (e.key.keysym.mod & KMOD_CTRL)) {
                printf("Quit.\n");
                return false;
            }
            else if (e.key.keysym.sym == SDLK_t) {
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

    /*
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
    if (currentKeyStates[SDL_SCANCODE_F4]
        && (currentKeyStates[SDL_SCANCODE_LALT] || currentKeyStates[SDL_SCANCODE_RALT]))
    {
        printf("Force quit\n");
        //return false;
    }
    */

    game.OnUpdate(dt);
    SDL_RenderPresent(renderer);
    
    return true;
}

const int FPS = 60;
const int TICKS_PER_FRAME = 1000 / FPS;

int main(void) {
    SDL sdl;

    try {
        sdl.init();
    }
    catch (const std::runtime_error& ex) {
        printf("Failed to initialize SDL.\n%s\n", ex.what());
        return 1;
    }


    try {
        //loadMedia(sdl, game);
        Game game(sdl.window);
        bool running = true;
        Uint32 lastFrame = SDL_GetTicks();
        game.OnStart();

        while (running) {
            Uint32 current = SDL_GetTicks();
            double dt = (current - lastFrame) / 1000.0;
            lastFrame = current;
            running = Update(game, dt);

            const int updateTime = SDL_GetTicks() - lastFrame;
            if (updateTime < TICKS_PER_FRAME) {
                SDL_Delay(TICKS_PER_FRAME - updateTime);
            }
        }

        game.save();
    }

    catch (const std::runtime_error& ex) {
        printf("%s\n", ex.what());
        return 1;
    }


    return 0;


}
