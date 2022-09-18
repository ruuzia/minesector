#!/usr/bin/env sh

em++ src/*.cpp -Wall -o demo.js \
    -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2 \
    -sSDL2_IMAGE_FORMATS="['png']" -sNO_DISABLE_EXCEPTION_CATCHING \
    --preload-file images --preload-file fonts --preload-file sounds \
    "$@"
