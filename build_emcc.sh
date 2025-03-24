#!/usr/bin/env sh

set -xe

cd docs

### wasm compiling optimizations
## base:                     demo.wasm=3.0M, demo.js=393K
## -Os:                      demo.wasm=2.3M, demo.js=192K
## -fno-exceptions -fno-rtti demo.wasm=2.3M  demo.js=188K
## disable harfbuz           demo.wasm=1.7M  demo.js=188K

em++ ../src/anim.cpp ../src/color.cpp ../src/game.cpp ../src/texture.cpp \
    ../src/button.cpp ../src/font.cpp ../src/main.cpp ../src/text.cpp ../src/tile.cpp \
    -Wall -o demo.js \
    -D RUNTIME_BASE_PATH="" \
    -Os -fno-exceptions -fno-rtti \
    --use-port=sdl2 --use-port=sdl2_image:formats=png --use-port=sdl2_mixer \
    --use-port=./emscripten_sdl2_ttf.py \
    --preload-file ../assets@assets \
    -sEXPORTED_FUNCTIONS=_main,_save,_onClick,_onAltClick --js-library mine.js \
    -sSTACK_SIZE=1000000 \
    "$@"
