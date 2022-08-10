#include "text.h"

Text::Text(TTF_Font *font, std::string string_)
    : font(font), string(string_), color(0.f, 0.f, 0.f)
{
    scale = 1.0;
}

void Text::load() {
    tex.loadText(font, string.c_str(), color.as_sdl());
    tex.setScale(scale);
    loaded = true;
}

void Text::render() {
    if (!loaded) load();
    tex.render(x, y);
}
