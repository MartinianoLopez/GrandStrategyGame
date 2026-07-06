#!/bin/bash
source /home/omarti/Software/libs/emsdk/emsdk_env.sh

mkdir -p webAssembly

emcc src/main.cpp -Isrc \
  -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 \
  -s SDL2_IMAGE_FORMATS='["png","bmp","tga"]' \
  -s WASM=1 \
  -s ALLOW_MEMORY_GROWTH=0 \
  -s INITIAL_MEMORY=536870912 \
  -s FORCE_FILESYSTEM=1 \
  --preload-file assets \
  --exclude-file "assets/terrain/*" \
  --exclude-file "assets/terrainCustom/*" \
  --exclude-file "assets/terrainOld/*" \
  -o webAssembly/index.html

python3 /home/omarti/Software/libs/emsdk/upstream/emscripten/tools/file_packager.py \
  webAssembly/data2.data \
  --preload assets/terrainCustom@assets/terrainCustom \
  --js-output=webAssembly/data2.js

cd webAssembly && python3 -m http.server 8000