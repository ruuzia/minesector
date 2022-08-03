#ifndef BUTTON_H
#define BUTTON_H

#include "texture.h"


class Button {
public:
    Button(Texture *tex);
    Button() {
        background = nullptr;
    }
    virtual ~Button();

    const Texture* background;

    bool isMouseOver(int mouseX, int mouseY);

    virtual void onClick(SDL_Event *e) {}

    virtual void render() {
        background->render(x, y);
    }

    virtual int getWidth() const { return background->getWidth(); }
    virtual int getHeight() const { return background->getHeight(); }

    // Just leaving position public for now
    int x, y;
private:
};


class TextButton : public Button {
public:
    TextButton();
    ~TextButton();

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

    void setCenterPos(int x_, int y_) {
        x = x_ - text.getWidth() / 2 - borderWidth;
        y = y_ - text.getHeight() / 2 - borderWidth;
    }

    double scale;
private:
    Texture text;
    TTF_Font *font;
    const char* string;
    SDL_Color color;
    SDL_Color bgcolor;
    int borderWidth;
};

#endif
