#ifndef FRONTEND_H
#define FRONTEND_H
#ifdef __cplusplus
extern "C" {
#endif
#include <SDL_video.h>
#include <SDL_stdinc.h>
#include <stdbool.h>
extern bool openSaveReader(void);
extern Uint8 readByte(void);
extern bool openSaveWriter(void);
extern int writeByte(Uint8 value);
extern void closeSaveFile(void);
extern void frontend_init(char **arg);

#ifdef __cplusplus
}
#endif
#endif // FRONTEND_H
