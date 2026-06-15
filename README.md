# Magik and Clay | Name is subject to change
A WIP 2D game engine and prototype game built in C++20 with DirectX 11 and Win32. Built with philosophy from Handmade Hero, raylib and Quake 3. Layers are structured as a directed acyclic graph, with the intention of making cross platform porting easier.

## Usage
If you want to build the project you'll need CMake 3.12+ and I currently only support windows.
```
git clone https://github.com/jackboakes/magik-and-clay.git
cd magik-and-clay
cmake -B build
cmake --build build
```

## Dependencies
- [Handmade Math](https://github.com/HandmadeMath/HandmadeMath)
- [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)
- [stb_rect_pack](https://github.com/nothings/stb/blob/master/stb_rect_pack.h)
- [stb_truetype](https://github.com/nothings/stb/blob/master/stb_truetype.h)

## Credits

**Fonts:** 
- Alagard by Hewett Tsoi
- Romulus by Hewett Tsoi

**Sprites:**
- An edited version of [Cauldron](https://opengameart.org/content/alchemy-or-witchs-cauldrons)
