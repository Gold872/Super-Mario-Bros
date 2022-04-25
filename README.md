
# Super Mario Bros

A recreation of the Super Mario Bros game from the Nintendo NES using C++ and the SDL2 Library

## Compiling

The following libraries are required to compile and run this project
- [SDL2](https://www.libsdl.org/download-2.0.php)
- [SDL2_Image](https://www.libsdl.org/projects/SDL_image/)
- [SDL2_TTF](https://www.libsdl.org/projects/SDL_ttf/release/)

Once you have the zip files extracted, run this command

```bash
g++ -O1 -I"src/*" -I"res/*" -I"include/*" -L"<SDL2 Location>" -L"<SDL2_image Location>" -L"<SDL2_TTF Location" -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
```

## How it Works

### The Entities
- The Entities in this game are creating using an Entity Component System
    - Instead of having one Entity class that every Entity inherits, an Entity is a class that has nothing, but you are able to attatch components to it. Such components include:
        - Position Component
        - Texture Component
        - Moving Component
    - These Components get used by the Systems, where they find Entities with certain components, and update the Entities properly using the data from the Components.

### The Levels
* The Levels were created with the help of a program called [Tiled Map Editor](https://www.mapeditor.org/)

* After creating the Map in the Tiled Editor, they are exported as a CSV file, and then get read by the Map class, and using the IDs from the Map, the Entities get created with their needed components.
## Roadmap

- Finish creating the methods for loading all of the enemies
    - Bowser
    - Koopa Paratroopa
    - Fire Bar

- Add in the sounds

- Finish all of the Levels

- Add a menu screen


## Special Thanks
People that have been a huge help in developing this project with their amazing knowledge and skills
 - [Killme](https://github.com/killme)
 - [lemur68](https://github.com/friedkeenan/)
