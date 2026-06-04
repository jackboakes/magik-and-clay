#pragma once

#include <filesystem>
#include <array>

#include "camera/camera.h"
#include "renderer/renderer.h"



enum class TileKind
{
    None = -1,
    Grass_1,
    Grass_2,
    Grass_3
};

struct Tile
{
    TileKind kind;
    Vec2S32 position;
};

static constexpr int g_TileSize { 16 };
static constexpr int g_TileMapWidth { 160 };
static constexpr int g_TileMapHeight { 90 };
static Tile g_TileMap[g_TileMapWidth][g_TileMapHeight];
static Texture g_TileTextures[4]; // one per TileKind


enum class GolemState
{
    Idle,
    Pathing,
    Harvesting,
    Depositing
};

enum class EntityKind
{
    None = 0,
    Golem,
    Crop,
    Cauldron,
    Item
};

enum EntityFlags : uint8_t
{
    None = 0,
    Drawable = 1 << 0,
    Collidable = 1 << 1,
    Harvestable = 1 << 2 
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

static constexpr int g_MaxEntities { 1024 };

struct EntityHandle
{
    uint64_t id { 0 };
    size_t index { 0 };
};

struct Entity
{
    EntityHandle handle;
    EntityKind kind { EntityKind::None };
    uint8_t flags { EntityFlags::None };

    Texture texture;

    Vec2F32 position;
    Vec2F32 targetPosition;
    float speed; 
    GolemState golemState { GolemState::Idle };

    std::array<SpriteAnimation, 2> animations;
    size_t animationIdx { 0 };
    uint32_t animationTicks;

    uint32_t growthTicks;
    bool harvestable { false };

    uint32_t collisionWidth { 1 };  // width in tiles
    uint32_t collisionHeight { 1 }; // height in tiles


    // Ad-hoc farming
    bool cropTaken { false };
    EntityHandle cropTargetHandle { 0 };
    EntityHandle heldItem { 0 };

    std::vector<Vec2S32> path;

    inline bool HasFlag(uint32_t flag) const
    {
        return (flags & flag) != 0;
    }

    inline void AddFlag(uint32_t flag)
    {
        flags |= flag;
    }

    inline void RemoveFlag(uint32_t flag)
    {
        flags &= ~flag;
    }
};

struct GameState
{
    float virtualScreenWidth;
    const float virtualScreenHeight { 360.f };

    uint64_t tick;

    Camera camera;

    uint64_t nextEntityId { 1 };
    std::array<Entity, g_MaxEntities> entities;
    EntityHandle activeEntityHandle { 0 };

    // assets

    Font font1;
    Font font2;

    Texture golemTexture;
    Texture interactableTexture;
    Texture daisyItemTexture;
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