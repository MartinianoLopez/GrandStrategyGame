# Grand Strategy Game

![screenshot](/docs/screenshot-2026-05-07_16-37-54.png)

## Game Requirements

- Full 2D
- Reliable
- Scalable

## To Do

- make a main menu
- Convert frontier pixels to lines
- Improve troop sistem
- Make a basic game dinamic
- Make ia countries

## Setup

To run this project you need the original EU4 assets. Copy the following files from your EU4 installation folder into the `assets/` directory:

```text
assets/
├── provinces.bmp
├── terrain.bmp
├── heigthmap.bmp
├── definition.csv
├── ProvinceIdtoTroops.csv
├── 00_countries.txt
├── provinces/
│   └── 1-UppLand.txt
└── countries/
    └── Sweden.txt
```

this will be replaced in the future with my own assets

## Second version ideas

- make clases instead of list for all the game data
- use json to load the assets.
- use CMake + SDL3 + OpenGL + RmlUi
