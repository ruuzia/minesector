#include <SDL_render.h>
#include <SDL_ttf.h>
#include <string>
#include <stdio.h>
#include "game.h"
#include "app.h"
#include "color.h"
#include "frontend.h"

static SDL_RWops *rw;
static std::string* getSaveFile(void) {
    char* dir = SDL_GetPrefPath("grassdne", "minesector");
    if (dir == NULL) {
        fprintf(stderr, "Error getting the pref dir\n");
        return NULL;
    }
    std::string *file = new std::string(std::string(dir)+Save::FILE);
    SDL_free(dir);
    return file;
}

bool openSaveReader(void) {
    std::string *file = getSaveFile();
    if (file == NULL) return false;
    rw = SDL_RWFromFile(file->c_str(), "r+b");
    fflush(stdout);
    delete file;
    if (rw == NULL) {
        fprintf(stderr, "Error opening save file for reading (%s)\n", SDL_GetError());
        return false;
    }
    return rw != NULL;
}
bool openSaveWriter(void) {
    std::string *file = getSaveFile();
    if (file == NULL) return false;
    rw = SDL_RWFromFile(file->c_str(), "w+b");
    delete file;
    if (rw == NULL) {
        fprintf(stderr, "Error opening save file for writing (%s)\n", SDL_GetError());
        return false;
    }
    return rw != NULL;
}
Uint8 readByte(void) {
    return SDL_ReadU8(rw);
}
int writeByte(Uint8 value) {
    return SDL_WriteU8(rw, value);
}
void closeSaveFile(void) {
    SDL_RWclose(rw);
    rw = NULL;
}

void frontend_init(char **arg) {
    (void)arg;
}
