#!/usr/bin/env sh

# Until I can get cmake working with emscripten

em++ src/*.cpp -Wall -o demo.js -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -s USE_SDL_MIXER=2 -s -sSDL2_IMAGE_FORMATS="['png']" -sNO_DISABLE_EXCEPTION_CATCHING --preload-file images --preload-file fonts --preload-file sounds
