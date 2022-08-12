#ifndef FONT_H
#define FONT_H

#include "SDL_ttf.h"
#include <string>
#define BASE_FONT_SIZE 40

class Font {
public:
    Font();
    Font(std::string path, int size = BASE_FONT_SIZE) {
        load(path, size);
    }
    ~Font();
    void load(std::string path, int size);

    [[nodiscard]] TTF_Font *raw() const { SDL_assert(font != nullptr); return font; }

private:
    TTF_Font *font;
};

#endif
