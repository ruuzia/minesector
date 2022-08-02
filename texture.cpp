#include "texture.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define OK true
#define FAIL false

Texture::Texture() {
    texture = nullptr;
    width = 0;
    height = 0;
    //pos.x = 0;
    //pos.y = 0;
}

Texture::~Texture() {
    free();
}

void Texture::render(SDL_Renderer *renderer, int x, int y, SDL_Rect *clip) const {
    const SDL_Rect dstrect = {
        .x = x,
        .y = y,
        .w = width,
        .h = height,
    };
    SDL_RenderCopy(renderer, texture, clip, &dstrect);
}

void Texture::free() {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
        width = 0;
        height = 0;
    }
}

bool Texture::loadFile(SDL_Renderer *renderer, const char* path) {
    // Free any existing texture
    free();

    // Directly load image to texture
    texture = IMG_LoadTexture(renderer, path);
    if (texture == NULL) {
        printf("Unable to create texture from image %s. SDL_image error: %s\n", path, IMG_GetError());
        return FAIL;
    }

    // Get width and height of texture
    if (SDL_QueryTexture(texture, NULL, NULL, &width, &height) < 0) {
        printf("Unable to query texture. SDL error: %s\n", SDL_GetError());
        return FAIL;
    }


    return OK;
}

bool Texture::loadText(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color) {
    free();

    // Need to create temp surface and convert to texture
    SDL_Surface *tmpSurface = TTF_RenderText_Solid(font, text, color);
    if (tmpSurface == NULL) {
        printf("Unable to load text. SDL_ttf error: %s\n", TTF_GetError());
        return FAIL;
    }

    texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
    if (texture == NULL) {
        printf("Unable to create texture from text. SDL error: %s\n", SDL_GetError());
        return FAIL;
    }
    width = tmpSurface->w;
    height = tmpSurface->h;

    SDL_FreeSurface(tmpSurface);

    return OK;
}

