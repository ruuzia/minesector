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

    bool loadFile(const char* file);

    // Overload to also set width and height
    bool loadFile(const char* file, int width, int height) {
        if (loadFile(file) == FAIL) return FAIL;
        setSize(width, height);
        return OK;
    }

    bool loadText(TTF_Font *font, const char* text, SDL_Color color);

    void free();

    // Render at a point on screen
    void render(int x, int y, SDL_Rect *clip = NULL, double angle = 0.0, SDL_Point *center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE) const;

    void renderPart(int x, int y, const SDL_Rect *rect) const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void setSize(int w, int h) {
        width = w;
        height = h;
    }

    void setScale(double f) {
        width = imgWidth * f;
        height = imgHeight * f;
    }

    void setMultColor(double r, double g, double b) {
        SDL_SetTextureColorMod(texture, (Uint8)r*255, (Uint8)r*255, (Uint8)r*255);
    }

    bool loaded() const {
        return texture != nullptr;
    }

    //SDL_Point pos;

private:
    int width;
    int height;
    int imgWidth, imgHeight;
    SDL_Texture *texture;
};

#endif
