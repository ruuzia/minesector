#include "font.h"
#include "app.h"

Font::Font() {
    font = nullptr;
}

void Font::load(std::string path, int size) {
    font = TTF_OpenFont((Sim.runtimeBasePath + path).c_str(), size);
    if (font == nullptr) {
        fprintf(stderr, "Failed to load font! SDL_ttf error: %s\n", TTF_GetError());
        exit(1);
    }
}
Font::~Font() {
    if (font != nullptr) {
        TTF_CloseFont(font);
    }
}

