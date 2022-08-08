#include "text.h"

Text::Text(TTF_Font *font, std::string string_, SDL_Color color_)
    : font(font), string(string_), color(color_)
{
    scale = 1.0;
}

void Text::load() {
    tex.loadText(font, string.c_str(), color);
    tex.setScale(scale);
    loaded = true;
}

void Text::render() {
    if (!loaded) load();
    tex.render(x, y);
}
