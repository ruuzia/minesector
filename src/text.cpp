#include "text.h"


Text::Text(TTF_Font * font, std::string string_, Color color)
    : font(font), string(string_), color(color)
{
    scale = 1.0;
}

void Text::load() {
    if (loaded) return;
    tex.loadText(font, string.c_str(), color.as_sdl());
    tex.setScale(scale);
    loaded = true;
}

void Text::render() {
    load();
    tex.render(x, y);
}
