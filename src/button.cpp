#include "button.h"
#include "color.h"

Button::Button(Texture *tex) : background(tex) {
}

Button::~Button() {}

bool Button::isMouseOver(int mouseX, int mouseY) const {
    // Rather verbose which is fine.
    const int sideLeft = x;
    const int sideTop = y;
    const int sideRight = x + getWidth();
    const int sideBottom = y + getHeight();

    if (mouseX < sideLeft || mouseX > sideRight) {
        return false;
    }
    if (mouseY < sideTop || mouseY > sideBottom) {
        return false;
    }

    return true;
}


TextButton::TextButton(std::string string, Color color) : Button(), text(string, color) {
    scale = 1.0;
    borderWidth = 15;
    bgcolor = {HEX(0xF0F0F0), 0xE0};
}

int TextButton::getWidth() const {
    SDL_assert(text.loaded);
    return text.getWidth() + borderWidth * 2;
}

int TextButton::getHeight() const {
    SDL_assert(text.loaded);
    return text.getHeight() + borderWidth * 2;
}

TextButton::~TextButton() {}

void TextButton::load() {
    text.load();
}

void TextButton::render() {
    if (!text.loaded) {
        load();
    }
    SDL_SetRenderDrawColor(renderer, bgcolor.r, bgcolor.g, bgcolor.b, bgcolor.a);
    SDL_Rect fillRect = {
        x, y,
        text.getWidth() + 2 * borderWidth,
        text.getHeight() + 2 * borderWidth
    };
    SDL_RenderFillRect(renderer, &fillRect);
    text.render();
}

