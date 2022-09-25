#include "button.h"
#include "color.h"

Button::Button(Texture *tex) : background(tex), hidden(false) {
    hidden = false;
    onclick = nullptr;
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


TextButton::TextButton(TTF_Font *font, std::string string, Color color)
    : Button()
    , text(font, string, color)
    , bgcolor(0xF0F0F0, 0.85)
    , hoverbg(0xFFFFFF, 0.95)
{
    scale = 1.0;
    borderWidth = 15;
    active = false;
    hidden = false;
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
    (active ? hoverbg : bgcolor).draw();
    SDL_Rect fillRect = {
        x, y,
        text.getWidth() + 2 * borderWidth,
        text.getHeight() + 2 * borderWidth
    };
    SDL_RenderFillRect(renderer, &fillRect);
    text.render();
}

void TextButton::mouseEnter() {
    active = true;
}
void TextButton::mouseLeave() {
    active = false;
}

