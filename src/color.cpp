#include "color.h"

Color Color::operator*(const Color& rhs) const {
    return Color(r * rhs.r, g * rhs.g, b * rhs.b, a * rhs.a);
}

Color& Color::operator*=(const Color& rhs) {
    r *= rhs.r;
    g *= rhs.g;
    b *= rhs.b;
    a *= rhs.a;

    return *this;
}

const SDL_Color Color::as_sdl() {
    SDL_Color sdl_color {PtoU8(r), PtoU8(g), PtoU8(b), PtoU8(a)};
    return sdl_color;
}
