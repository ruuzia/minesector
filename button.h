#ifndef BUTTON_H
#define BUTTON_H

#include "texture.h"


class Button {
public:
    Button(Texture *tex);
    ~Button();

    Texture* texture;

    bool isMouseOver(int mouseX, int mouseY);

    virtual void onClick(SDL_Event *e) {}

    void render(SDL_Renderer *renderer) const {
        texture->render(renderer, x, y);
    }

    // Just leaving position public for now
    int x, y;
private:
};

#endif
