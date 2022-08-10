#ifndef COLOR_H
#define COLOR_H
#include "SDL.h"
#include "common.h"

// Use with hexadecimal literals for hex colors
// Eg. HEX(0xFA8005) -> 0xFA, 0x80, 0x05
#define HEX(N) Uint8(N >> 0x10), Uint8((N >> 0x08) & 0xFF), Uint8(N & 0xFF)

#define U8toP(u8) float((u8) / 255.f)
#define PtoU8(perc) Uint8(perc * 255)

// Color vales stored as real [0, 1]
// Float or double?
typedef float percent;

class Color {
public:
    // Lots of different constructor overloads for convenience
    // These should be inlined at compile time
    Color(percent r, percent g, percent b, percent a = 1.0) :
        r(r), g(g), b(b), a(a) {}

    Color(Uint8 r, Uint8 g, Uint8 b, percent a = 1.0)
        : Color(U8toP(r), U8toP(g), U8toP(b), a) {}

    Color(int hex, percent a = 1.0) : Color(HEX(hex), a) { }

    Color(SDL_Color c) : Color(c.r, c.g, c.b, U8toP(c.a)) {}


    // These do the same things as constructors
    void set(percent r_, percent g_, percent b_, percent a_ = 1.0) {
        r = r_;
        g = g_;
        b = b_;
        a = a_;
    }

    void set(Uint8 r, Uint8 g, Uint8 b, percent a = 1.0) {
        set(U8toP(r), U8toP(g), U8toP(b), a);
    }

    void set(int hex, percent a = 1.0) {
        set(HEX(hex), a);
    }

    void set(const Color &color) {
        r = color.r;
        g = color.g;
        b = color.b;
        a = color.a;
    }

    void setToRender() const {
        SDL_SetRenderDrawColor(renderer, PtoU8(r), PtoU8(g), PtoU8(b), PtoU8(a));
    }

    const SDL_Color as_sdl();

    Color operator*(const Color& rhs) const;
    Color& operator*=(const Color& rhs);

    // Leaving these public
    percent r, g, b, a;
};

#endif
