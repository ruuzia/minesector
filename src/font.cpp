#include "font.h"
#include <stdexcept>

Font::Font() {
    font = nullptr;
}

void Font::load(std::string path, int size) {
    font = TTF_OpenFont(path.c_str(), size);
    if (font == nullptr) {
        throw std::runtime_error("Failed to load font! SDL_ttf error: " + std::string(TTF_GetError()));
    }
}
Font::~Font() {
    TTF_CloseFont(font);
}

