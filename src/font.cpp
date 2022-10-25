#include "font.h"
#include <stdexcept>
#include "app.h"

Font::Font() {
    font = nullptr;
}

void Font::load(std::string path, int size) {
    font = TTF_OpenFont((Sim.runtimeBasePath + path).c_str(), size);
    if (font == nullptr) {
        throw std::runtime_error("Failed to load font! SDL_ttf error: " + std::string(TTF_GetError()));
    }
}
Font::~Font() {
    if (font != nullptr) {
        TTF_CloseFont(font);
    }
}

