# Grand Strategy Game

![screenshot](/docs/gamestate.png)

## Game Range

- Full 2D
- Reliable
- Scalable

## Game requierements

The game has to do everything that EU4 is capable to do, without entering in the more complex sistems, only the essentials to fullfill all the main activities.

## MVP To Do

- polish the UI
- complete the troop system
- make time management
- Make basic game functions:
diplomacy system, economy system
- Make inteligent countries

## next version ideas

- convert frontier pixels to lines
- impruve the game aesthetics 
- make the ui code into a txt config file wich is read every few renders to be able to change the ui like a css in real time.

## MVC Arquitecture 

- Model     =  World: controlls all the game data in one place
- View      =  Renderer: draws all the layers in one place
- Controler =  Events management: manege all the inputs in one place

## optimization issue 

  [map]
  setup: 0.0002 ms
  height: 0.001854 ms
  terrain: 0.000611 ms
  countries: 0.000441 ms                   
  provFront: 5.78817 ms                         (too slow and twice)
  countryFront1: 0.00018 ms
  countryFront2/marks/armies: 3.02832 ms        (too slow and twice)

  [map_second]
  setup: 0.000581 ms
  height: 0.000962 ms
  terrain: 0.000621 ms
  countries: 0.000411 ms                       
  provFront: 3.62776 ms                         (too slow and twice)
  countryFront1: 0.00012 ms
  countryFront2/marks/armies: 1.74251 ms        (too slow and twice)