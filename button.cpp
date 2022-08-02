#include "button.h"

Button::Button(Texture *tex) : texture(tex) {}

Button::~Button() {}

bool Button::isMouseOver(int mouseX, int mouseY) {
    // Rather verbose which is fine.
    const int sideLeft = x;
    const int sideTop = y;
    const int sideRight = x + texture->getWidth();
    const int sideBottom = y + texture->getHeight();

    if (mouseX < sideLeft || mouseX > sideRight) {
        return false;
    }
    if (mouseY < sideTop || mouseY > sideBottom) {
        return false;
    }

    return true;
}


