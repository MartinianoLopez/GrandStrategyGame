#!/bin/bash
source /home/omarti/Software/libs/emsdk/emsdk_env.sh

mkdir -p webAssembly

emcc src/main.cpp -Isrc -Isrc/third_party \
-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 \
-s SDL2_IMAGE_FORMATS='["png","bmp","tga"]' \
-s WASM=1 \
-s ALLOW_MEMORY_GROWTH=0 \
-s INITIAL_MEMORY=536870912 \
-s FORCE_FILESYSTEM=1 \
--preload-file assets \
--exclude-file "assets/terrain/*" \
--exclude-file "assets/terrainOld/*" \
-o webAssembly/index.html

kill -9 $(lsof -t -i:8000) 2>/dev/null
cd webAssembly && python3 -m http.server 8000