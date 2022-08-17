#include "texture.h"
#include <SDL_rect.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#define OK true
#define FAIL false

Texture::Texture() {
    texture = nullptr;
    width = 0;
    height = 0;
    imgHeight = 0;
    imgWidth = 0;
    //pos.x = 0;
    //pos.y = 0;
}

Texture::~Texture() {
    free();
}

void Texture::render(int x, int y, SDL_Rect *clip, double angle, SDL_Point *center, SDL_RendererFlip flip) const {
    const SDL_Rect dstrect = { x, y, width, height };
    SDL_RenderCopyEx(renderer, texture, clip, &dstrect, angle, center, flip);
}

void Texture::renderPart(int x, int y, const SDL_Rect *rect, bool stretchSource) const {
    const SDL_Rect srcrect = {
        rect->x,
        rect->y,
        rect->w * (int) ((float)imgWidth / (float)width),
        rect->h * (int) ((float)imgHeight / (float)height),
    };

    const SDL_Rect dstrect = { x + rect->x, y + rect->y, rect->w, rect->h };

    SDL_RenderCopy(renderer, texture, stretchSource ? NULL : &srcrect, &dstrect);
}

void Texture::free() {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
        width = 0;
        height = 0;
    }
}

void Texture::loadFile(std::string& path) {
    // Free any existing texture
    free();

    // Directly load image to texture
    texture = IMG_LoadTexture(renderer, path.c_str());
    if (texture == nullptr) {
        throw std::runtime_error("Unable to create texture from image " + path + ". SDL_image error: " + std::string(IMG_GetError()));
    }

    // Get width and height of texture
    if (SDL_QueryTexture(texture, nullptr, nullptr, &width, &height) < 0) {
        throw std::runtime_error("Unable to query texture. SDL error: " + std::string(SDL_GetError()));
    }

    imgWidth = width;
    imgHeight = height;
}

//void Texture::loadText(Font &font, const char *text, SDL_Color color) {
void Texture::loadText(TTF_Font *font, const char *text, SDL_Color color) {
    free();

    //TTF_Font *fnt = font.raw();
    // Need to create temp surface and convert to texture
    SDL_Surface *tmpSurface = TTF_RenderText_Solid(font, text, color);
    if (tmpSurface == nullptr) {
        throw std::runtime_error("Unable to load text. SDL_ttf error: " + std::string(TTF_GetError()));
    }

    texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
    if (texture == nullptr) {
        throw std::runtime_error("Unable to create texture from text. SDL error: " + std::string(SDL_GetError()));
    }
    width = tmpSurface->w;
    height = tmpSurface->h;
    imgWidth = width;
    imgHeight = height;

    SDL_FreeSurface(tmpSurface);
}

