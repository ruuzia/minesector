#ifndef TEXT_H
#define TEXT_H

#include <SDL_ttf.h>
#include <SDL.h>
#include <string>
#include "texture.h"

class Text {
public:
    Text(TTF_Font *font, std::string string_={}, SDL_Color color_ = {0,0,0,0});
    ~Text() = default;

    void render();
    void load();

    void setFont(TTF_Font *font_) {
        font = font_;
    }
    void setString(const char* string_) {
        string = string_;
    }
    void setColor(Uint8 r, Uint8 g, Uint8 b) {
        color.r = r;
        color.g = g;
        color.b = b;
    }

    int getWidth() const;
    int getHeight() const;

    int x, y;

    float scale;

    TTF_Font *font;
    std::string string;
    SDL_Color color;
private:
    Texture tex;
};

#endif
