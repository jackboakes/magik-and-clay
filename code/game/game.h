#pragma once

#include <filesystem>
#include <array>

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


enum class GolemState
{
    Idle,
    Pathing,
    Harvesting,
    Depositing
};

enum class EntityType
{
    Golem
};

struct SpriteAnimation
{
    uint32_t frameCount;
    uint32_t currentFrame;
    uint32_t frameWidth;
    uint32_t frameHeight;
    uint32_t xOffset;
    uint32_t yOffset;
    // TODO:: add frame tick
};


struct Entity
{
    EntityType type;
    Texture texture;

    HMM_Vec2 position;

    std::array<SpriteAnimation, 2> animations;
    // TODO:: temp to show animation for now
    bool carryingItem;
};

struct GameState
{
    float virtualScreenWidth;
    const float virtualScreenHeight { 360.f };

    Camera camera;

    std::array<Entity, 32> entities;

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
    void Update(float deltaTime);
    void DrawFrame(float deltaTime);
    void UpdateAndDrawFrame(float deltaTime);
}