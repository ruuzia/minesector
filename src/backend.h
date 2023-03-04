#ifndef BACKEND_H
#define BACKEND_H
#ifdef __cplusplus
extern "C" {
#endif
#include <SDL_video.h>
#include <SDL_stdinc.h>

void save(void);
void onClick(int x, int y);
void onAltClick(int x, int y);
void quit(void);

#ifdef __cplusplus
}
#endif
#endif // FRONTEND_H
