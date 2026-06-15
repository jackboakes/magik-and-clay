# Magik and Clay

A WIP 2D game engine and prototype game built in C++20 with DirectX 11 for the rendering layer and Win32 for the OS platform layer. Built with philosophies from Handmade Hero, raylib, and Quake III. Layers are structured as a directed acyclic graph, with the intention of making cross platform porting easier.

Here's what I've implemented so far:
- Sprite rendering
- Sprite animations
- Font rendering
- Draw calls batched by texture
- Draw calls are batched into two passes: one for world space and one for screen space
- Depth testing to determine sprite draw order
- Camera with zoom to cursor and panning via WASD
- Entity system
- Left click to pick an entity and right click to move via A*
- Tiles and entities are culled if not visible to the camera
- Windowed fullscreen via Alt + Enter
 - & many more
 
## Preview
<img width="1919" height="1079" alt="game engine protoype as of 150626" src="https://github.com/user-attachments/assets/33790b74-fa01-4bec-bc93-7caf3ba46141" />

## Usage
If you want to build the project you'll need to be on windows with CMake 3.12+.
```
git clone https://github.com/jackboakes/magik-and-clay.git
cd magik-and-clay
cmake -B build
cmake --build build
```
After building, the executable is located in the root folder.  

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
