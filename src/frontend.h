#ifndef FRONTEND_H
#define FRONTEND_H
#ifdef __cplusplus
extern "C" {
#endif
#include <SDL_video.h>
#include <SDL_stdinc.h>

extern bool openSaveReader(void);
extern Uint8 readByte(void);
extern bool openSaveWriter(void);
extern int writeByte(Uint8 value);
extern void closeSaveFile(void);

#ifdef __cplusplus
}
#endif
#endif // FRONTEND_H
