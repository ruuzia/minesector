#ifndef COMMON_H
#define COMMON_H
#include "SDL_render.h"

#define OK true
#define FAIL false

// TODO: move somewhere else
#define SECONDS() (SDL_GetTicks() * 0.001)

// Global renderer!
extern SDL_Renderer *renderer;

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

#endif
