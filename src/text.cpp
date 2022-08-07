#include "text.h"

Text::Text(TTF_Font *font, std::string string_, SDL_Color color_)
    : font(font), string(string_), color(color_)
{
}

void Text::load() {
    tex.loadText(font, string.c_str(), color);
    tex.setScale(scale);
}

void Text::render() {
    tex.render(x, y);
}
