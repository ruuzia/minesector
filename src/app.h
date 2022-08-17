#ifndef COMMON_H
#define COMMON_H
#include "SDL_render.h"

#define SECONDS() (SDL_GetTicks() * 0.001)

extern SDL_Renderer *renderer;

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

class App {
public:
    App();
    ~App();

    bool isFullscreen;
    void init();
    SDL_Window *window;
};

extern App Sim;

#endif
