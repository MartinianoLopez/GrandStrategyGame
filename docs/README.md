# Kingdoms, Lands and Seas — Grand Strategy Game
 
Kingdoms, Lands and Seas aims to be a minimalistic, easy-to-play grand strategy game for players new to the genre, who often struggle to get into this type of game. The game is set in the year 1444 AC, allowing the player to play as any country in the world during this era, fighting with other countries to become powerful — or even rule the world.
 
## Technology Choices
 
The game is built using only **C++**, chosen for its efficiency in resource management, and **SDL2**, used for window management, basic rendering, and input handling.
 
## Architecture Principles
 
The game is designed to be data driven in every possible way, allowing anyone to modify its visuals simply by changing an image, a txt file, or a json file. This includes the UI, which is fully defined in json and compiled at runtime into UI objects, letting developers add or remove static UI elements at will.
 
## Assets
 
Development started with placeholder assets sourced from Europa Universalis IV. Since then, a significant effort has gone into replacing each of them with hand-polished, open source alternatives.


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
| **Model** | `World` | Is where all the data lives in diferent structures |
| **View** | `Renderer` | Draws all data to the screen |

---

## Screenshots on development stage

| | |
|---|---|
| ![](1.png) | ![](2.png) |
| ![](3.png) | ![](4.png) |
| ![](5.png) | ![](6.png) |

---