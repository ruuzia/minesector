#ifndef TEXT_H
#define TEXT_H

#include <SDL_ttf.h>
#include <SDL.h>
#include <string>
#include "texture.h"
#include "color.h"

class Text {
public:
    Text(TTF_Font *font = nullptr, std::string string_={});
    ~Text() = default;

    void render();
    void load();

    void setFont(TTF_Font *font_) {
        font = font_;
        loaded = false;
    }
    void setString(std::string string_) {
        string = string_;
        loaded = false;
    }
    void setColor(const Color& color_) {
        color.set(color_);
        loaded = false;
    }
    //void setColor(const Color&& color_) {
    //    setColor(color_);
    //}
    void setScale(double scale_) {
        scale = scale_;
        // Update texture if loaded
        if (loaded) tex.setScale(scale);
    }

    [[nodiscard]] int getWidth() const { return tex.getWidth(); }
    [[nodiscard]] int getHeight() const { return tex.getHeight(); }

    int x, y;

    bool loaded = false;
    TTF_Font *font;
    std::string string;
    Color color;
    float scale;

private:
    Texture tex;
};

#endif
