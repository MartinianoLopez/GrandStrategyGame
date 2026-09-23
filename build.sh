#!/bin/bash
set -euo pipefail

source /home/omarti/Software/libs/emsdk/emsdk_env.sh

mkdir -p webAssembly

em++ src/main.cpp -Isrc -Isrc/third_party \
-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 \
-s SDL2_IMAGE_FORMATS='["png","bmp","tga"]' \
-s WASM=1 \
-s ALLOW_MEMORY_GROWTH=1 \
-s INITIAL_MEMORY=536870912 \
-s FORCE_FILESYSTEM=1 \
--preload-file assets \
-o webAssembly/index.html

if server_pids=$(lsof -t -i:8001); then
    kill $server_pids
fi
cd webAssembly && python3 -m http.server 8001
