#include "app.h"

// I need this on Windows for some reason
#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <cstdio>

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

#include "texture.h"
#include "game.h"

constexpr int SCREEN_WIDTH  = 640 * 1.5;
constexpr int SCREEN_HEIGHT = 480 * 1.5;

SDL_Renderer *renderer{};

static bool running = true;

App::App() : isFullscreen{}, window{} {}

App::~App() {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;

        SDL_DestroyWindow(window);
        window = nullptr;

        IMG_Quit();
        TTF_Quit();
        Mix_Quit();
        SDL_Quit();
}

void App::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL could not initialize! SDL Error: " + std::string(SDL_GetError()));
    }

    window = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) throw std::runtime_error("Unable to create window. SDL Error: " + std::string(SDL_GetError()));

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
        throw std::runtime_error("Unable to create accelerated renderer. SDL Error: " + std::string(SDL_GetError()));

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);



    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
        throw std::runtime_error("SDL_image could not initialize: " + std::string(IMG_GetError()));

    if (TTF_Init() == -1) {
        throw std::runtime_error("SDL_ttf could not initialize: " + std::string(TTF_GetError()));
    }

    if (Mix_OpenAudio(/* Frequency */ 44100, MIX_DEFAULT_FORMAT, /* Channels */ 2, /* Chunksize */ 2048) < 0) {
        throw std::runtime_error("SDL_mixer count not initialize: " + std::string(Mix_GetError()));
    }

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
}

constexpr int FPS = 60;
constexpr int TICKS_PER_FRAME = 1000 / FPS;

Uint32 lastFrame;

// I need to use an event filter to support updating *while* resizing on windows
// Because Windows like to block the main thread
static int event_filter(void *game, SDL_Event *e) {
    if (e->type == SDL_WINDOWEVENT && e->window.event == SDL_WINDOWEVENT_RESIZED) {
        Uint32 current = SDL_GetTicks();

        if (current - lastFrame >= TICKS_PER_FRAME) {
            double dt = (current - lastFrame) / 1000.0;
            lastFrame = current;

            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_RenderClear(renderer);
                
            //static_cast<Game *>(game)->positionItems();
            static_cast<Game *>(game)->OnUpdate(dt);

            SDL_RenderPresent(renderer);
            printf("Window resize!\n");
        }
    }
    // Don't consume some keybinds
    if ((e->type == SDL_KEYDOWN
            && e->key.keysym.sym == SDLK_r
            && (e->key.keysym.mod & KMOD_CTRL))
        ) return 0;
    // Put on SDL_PollEvent queue
    return 1;
}

Color bgColor = 0xE0E0E0;
Game *game;

static void mainloop() {
    Uint32 current = SDL_GetTicks();
    double dt = (current - lastFrame) / 1000.0;
    lastFrame = current;

    bgColor.draw();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderClear(renderer);

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
            running = false;
            break;

        case SDL_KEYDOWN:
            // Force quit with Alt + F4
            if (e.key.keysym.sym == SDLK_F4 && (e.key.keysym.mod & KMOD_ALT)) {
#ifdef __EMSCRIPTEN__
                emscripten_cancel_main_loop();
#endif
                running = false;
                // Exit immediately, don't save game
                return;
            }
            else if (e.key.keysym.sym == SDLK_q && (e.key.keysym.mod & KMOD_CTRL)) {
#ifdef __EMSCRIPTEN__
                emscripten_cancel_main_loop();
#endif
                running = false;
            }
            else if (e.key.keysym.sym == SDLK_t) {
                printf("dt: %f\n", dt);
            }
            else if (e.key.keysym.sym == SDLK_F11) {
                if (Sim.isFullscreen) {
                    Sim.isFullscreen = false;
                    SDL_SetWindowFullscreen(Sim.window, 0);
                    SDL_SetWindowSize(Sim.window, SCREEN_WIDTH, SCREEN_HEIGHT);
                }
                else {
                    Sim.isFullscreen = true;
                    SDL_SetWindowFullscreen(Sim.window, SDL_WINDOW_FULLSCREEN);
                }
            }
            break;
        
        case SDL_MOUSEBUTTONDOWN:
            game->onMouseButtonDown(e.button);
            break;

        case SDL_MOUSEBUTTONUP:
            game->onMouseButtonUp(e.button);
            break;

        case SDL_MOUSEMOTION:
            game->onMouseMove(e.motion);
            break;
        }
    }

    game->OnUpdate(dt);
    SDL_RenderPresent(renderer);

    game->save();
}

App Sim;

int main(void) {
    try {
        Sim.init();
    }
    catch (const std::runtime_error& ex) {
        printf("Failed to initialize SDL.\n%s\n", ex.what());
        return 1;
    }


    try {
        Game _game(Sim.window);
        game = &_game;
        lastFrame = SDL_GetTicks();
        _game.OnStart();

        SDL_SetEventFilter(event_filter, &_game);

#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(mainloop, 0, 1);
        //emscripten_set_main_loop_arg(mainloop, &game, 0, 1);
#else
        while (running) {
            mainloop();

            const int updateTime = SDL_GetTicks() - lastFrame;
            if (updateTime < TICKS_PER_FRAME) {
                SDL_Delay(TICKS_PER_FRAME - updateTime);
            }
        }
#endif

    }

    catch (const std::runtime_error& ex) {
        printf("%s\n", ex.what());
        return 1;
    }


    return 0;


}
