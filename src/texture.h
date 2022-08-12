#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdexcept>

#include "SDL_rect.h"
#include "SDL_ttf.h"
#include "SDL_image.h"

#include "common.h"


class Texture
{
public:
    Texture();
    ~Texture();

    void loadFile(std::string& path);

    // Overload to also set w and h
    void loadFile(std::string& file, int w, int h) {
        loadFile(file);
        setSize(w, h);
    }

    //void loadText(Font &font, const char* text, SDL_Color color);
    void loadText(TTF_Font *font, const char* text, SDL_Color color);

    void free();

    // Render at a point on screen
    void render(int x, int y, SDL_Rect *clip = nullptr, double angle = 0.0, SDL_Point *center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE) const;

    void renderPart(int x, int y, const SDL_Rect *rect) const;

    [[nodiscard]] int getWidth() const { return width; }
    [[nodiscard]] int getHeight() const { return height; }

    void setSize(int w, int h) {
        width = w;
        height = h;
    }

    void setScale(double f) {
        width = (int) (imgWidth * f);
        height = int (imgHeight * f);
    }

    void setMultColor(double r, double g, double b) {
        SDL_SetTextureColorMod(texture, (Uint8)r*255, (Uint8)g*255, (Uint8)b*255);
    }

    [[nodiscard]] bool loaded() const {
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
