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
    imgHeight = 0;
    imgWidth = 0;
    //pos.x = 0;
    //pos.y = 0;
}

Texture::~Texture() {
    free();
}

void Texture::render(int x, int y, SDL_Rect *clip, double angle, SDL_Point *center, SDL_RendererFlip flip) const {
    const SDL_Rect dstrect = {
        .x = x,
        .y = y,
        .w = width,
        .h = height,
    };
    SDL_RenderCopyEx(renderer, texture, clip, &dstrect, angle, center, flip);
}

void Texture::renderPart(int x, int y, const SDL_Rect *rect) const {
    const SDL_Rect srcrect = {
        .x = rect->x,
        .y = rect->y,
        .w = rect->w * (int) (imgWidth / (float)width), 
        .h = rect->h * (int) (imgHeight / (float)height),
    };

    const SDL_Rect dstrect = {
        .x = x + rect->x,
        .y = y + rect->y,
        .w = rect->w,
        .h = rect->h,
    };

    SDL_RenderCopy(renderer, texture, &srcrect, &dstrect);
}

void Texture::free() {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
        width = 0;
        height = 0;
    }
}

bool Texture::loadFile(const char* path) {
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

    imgWidth = width;
    imgHeight = height;

    return OK;
}

bool Texture::loadText(TTF_Font *font, const char *text, SDL_Color color) {
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
    imgWidth = width;
    imgHeight = height;

    SDL_FreeSurface(tmpSurface);

    return OK;
}

