# "Kingdoms, Lands and Seas" — Grand Strategy Game

Kingdoms, Lands and Seas aims to be a minimalistic, easy-to-play grand strategy game for players new to the genre, who often struggle to get into this type of game. The game will be set in the 13th century, allowing the player to play as any country in the world during the feudal era, moving forward through the colonization of America and later the rest of the world.

# Technology Choices

The project uses only C++17 and SDL2 for rendering. For the UI, I tried other libraries like IMGUI, RmlUi, and WebView, but none of them offered a simple way to build UI based on PNG textures. So I built a simple compiler that reads a `.txt` file and translates it into UI components. It also supports hot reload, so I can tweak the UI at runtime.

# The Game Engine

The idea going forward is to turn this into a game engine for building any other kind of grand strategy game. Right now all the data is interchangeable, including the UI layout and textures, but the map visuals and the UI's supporting logic (the actions elements perform on click, and the text/information they display) are still deeply hardcoded.

---

## Architecture


```
                ┌───────────────┐     
                │  CONTROLLER   │ 
                │ Input Handler │     
                └───────┬───────┘     
                        │             
                        ▼             
                ┌───────────────┐      ┌──────────────-┐
                │     MODEL     │ ---> │  SIMULATION   │
                │     World     │ <--- │    Updates    │
                └───────┬───────┘      └──────────────-┘
                        │             
                        ▼             
                ┌───────────────┐     
                │     VIEW      │     
                |   Rendering   |
                └──────────────-┘
```

| Layer | Class | Responsibility |
|-------|-------|----------------|
| **Controller** | `EventManager` | Handles user input |
| **Simulation** | `Simulation` | Mutates the model driven by time and game events |
| **Model** | `World` | God object with all the data but no logic |
| **View** | `Renderer` | Draws all data to the screen |

---

## Screenshots on development stage

| | |
|---|---|
| ![](1.png) | ![](2.png) |
| ![](3.png) | ![](4.png) |
| ![](5.png) | ![](6.png) |

---