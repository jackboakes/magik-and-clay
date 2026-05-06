#pragma once

#include <filesystem>

#include "camera/camera.h"
#include "renderer/renderer.h"

enum class TileType
{
    None = -1,
    Crop,
    Grass_1,
    Grass_2,
    Grass_3,
    Cauldron
};

struct Tile
{
    TileType type;
};

static constexpr int g_TileSize { 16 };
static constexpr int g_TileMapWidth { 160 };
static constexpr int g_TileMapHeight { 90 };
static Tile g_TileMap[g_TileMapWidth][g_TileMapHeight];
static Texture g_TileTextures[5]; // one per TileType

struct GameState
{
    float virtualScreenWidth;
    const float virtualScreenHeight { 360.f };

    Camera camera;

    // assets

    Font font1;
    Font font2;
};

static GameState gameState { 0 };

namespace Game
{
    // TODO:: Ad-hoc tilemap loading
    void LoadTileMap(std::filesystem::path path);

    void Init();
    void Update();
    void DrawFrame();
    void Run();
}