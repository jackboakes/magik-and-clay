#pragma once

#include <filesystem>
#include <array>

#include "camera/camera.h"
#include "renderer/renderer.h"



enum class TileType
{
    None = -1,
    Grass_1,
    Grass_2,
    Grass_3
};

struct Tile
{
    TileType type;
    Vec2S32 position;
};

static constexpr int g_TileSize { 16 };
static constexpr int g_TileMapWidth { 160 };
static constexpr int g_TileMapHeight { 90 };
static Tile g_TileMap[g_TileMapWidth][g_TileMapHeight];
static Texture g_TileTextures[4]; // one per TileType


enum class GolemState
{
    Idle,
    Pathing,
    Harvesting,
    Depositing
};

enum class EntityType
{
    None = 0,
    Golem,
    Crop,
    Cauldron
};

struct SpriteAnimation
{
    uint32_t frameCount { 1 };
    uint32_t currentFrame;
    uint32_t frameWidth;
    uint32_t frameHeight;
    uint32_t xOffset;
    uint32_t yOffset;
    /*
    The number of ticks that advance an animations frame.
    1 means one animation frame per tick, 
    so at 60 ticks/sec that's 60 animation frames per second.
    30 means 30 frames before a new animation frame.
    */ 
    uint32_t frameAdvancement { 1 };
};


struct Entity
{
    EntityType type;
    Texture texture;

    Vec2F32 position;
    Vec2F32 targetPosition;
    float speed; 
    GolemState golemState { GolemState::Idle };

    std::array<SpriteAnimation, 2> animations;

    uint32_t animationTicks;
    uint32_t growthTicks;
    bool harestable { false };

    bool collidable { false };
    uint32_t collisionWidth { 1 };  // width in tiles
    uint32_t collisionHeight { 1 }; // height in tiles

    bool cropTaken { false };

    std::vector<Vec2S32> path;
};

struct GameState
{
    float virtualScreenWidth;
    const float virtualScreenHeight { 360.f };

    uint64_t tick;

    Camera camera;

    std::vector<Entity> entities;
    Entity* activeEntity;

    // assets

    Font font1;
    Font font2;

    Texture g_GolemTexture;
    Texture g_InteractableTexture;
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