#ifndef TEXTURE_H
#define TEXTURE_H

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_image.h"

#include "common.h"

class Texture
{
public:
    Texture();
    ~Texture();

    bool loadFile(SDL_Renderer *renderer, const char* file);

    // Overload to also set width and height
    bool loadFile(SDL_Renderer *renderer, const char* file, int width, int height) {
        if (loadFile(renderer, file) == FAIL) return FAIL;
        setSize(width, height);
        return OK;
    }

    bool loadText(SDL_Renderer *renderer, TTF_Font *font, const char* text, SDL_Color color);

    void free();

    // Render at a point on screen
    void render(SDL_Renderer *renderer, int x, int y, SDL_Rect *clip = NULL, double angle = 0.0, SDL_Point *center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE) const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void setSize(int w, int h) {
        width = w;
        height = h;
    }

    void setMultColor(double r, double g, double b) {
        SDL_SetTextureColorMod(texture, (Uint8)r*255, (Uint8)r*255, (Uint8)r*255);
    }

    //SDL_Point pos;

private:
    SDL_Texture *texture;
    int width;
    int height;
};

#endif
