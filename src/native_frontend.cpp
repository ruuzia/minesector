#include <SDL_render.h>
#include <SDL_ttf.h>
#include <string>
#include <stdio.h>
#include "game.h"
#include "app.h"
#include "color.h"
#include "frontend.h"

static SDL_RWops *rw;
std::string save_file_path;

bool openSaveReader(void) {
    rw = SDL_RWFromFile(save_file_path.c_str(), "r+b");
    if (rw == NULL) {
        fprintf(stderr, "Error opening save save_file_path for reading (%s)\n", SDL_GetError());
        return false;
    }
    return rw != NULL;
}
bool openSaveWriter(void) {
    rw = SDL_RWFromFile(save_file_path.c_str(), "w+b");
    if (rw == NULL) {
        fprintf(stderr, "Error opening save save_file_path for writing (%s)\n", SDL_GetError());
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
    if (arg[0] && strcmp(arg[0], "--save") == 0) {
        if (arg[1] == NULL) {
            throw std::runtime_error("--save requires additional argument");
        }
        save_file_path = arg[1];
    } else {
        char* dir = SDL_GetPrefPath("grassdne", "minesector");
        if (dir == NULL) {
            throw std::runtime_error("Error getting the pref dir");
        }
        save_file_path = std::string(std::string(dir)+Save::FILE);
        SDL_free(dir);
    }
    printf("Save path: %s\n", save_file_path.c_str());
}
