#pragma once

#include <filesystem>
#include <array>

#include "camera/camera.h"
#include "renderer/renderer.h"
#include "common/types.h"



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

static constexpr S32 g_TileSize { 16 };
static constexpr S32 g_TileMapWidth { 160 };
static constexpr S32 g_TileMapHeight { 90 };
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

enum EntityFlags : U8
{
    None = 0,
    Drawable = 1 << 0,
    Collidable = 1 << 1,
    Harvestable = 1 << 2 
};

struct SpriteAnimation
{
    U32 frameCount { 1 };
    U32 currentFrame;
    U32 frameWidth;
    U32 frameHeight;
    U32 xOffset;
    U32 yOffset;
    /*
    The number of ticks that advance an animations frame.
    1 means one animation frame per tick, 
    so at 60 ticks/sec that's 60 animation frames per second.
    30 means 30 frames before a new animation frame.
    */ 
    U32 frameAdvancement { 1 };
};

static constexpr U32 g_MaxEntities { 1024 };

struct EntityHandle
{
    U64 id { 0 };
    U64 index { 0 };
};

struct Entity
{
    EntityHandle handle;
    EntityKind kind { EntityKind::None };
    U8 flags { EntityFlags::None };

    Texture texture;

    Vec2F32 position;
    Vec2F32 targetPosition;
    F32 speed;
    GolemState golemState { GolemState::Idle };

    std::array<SpriteAnimation, 2> animations;
    size_t animationIdx { 0 };
    U32 animationTicks;

    U32 growthTicks;
    bool harvestable { false };

    U32 collisionWidth { 1 };  // width in tiles
    U32 collisionHeight { 1 }; // height in tiles


    // Ad-hoc farming
    bool cropTaken { false };
    EntityHandle cropTargetHandle { 0 };
    EntityHandle heldItem { 0 };

    std::vector<Vec2S32> path;

    inline bool HasFlag(U32 flag) const
    {
        return (flags & flag) != 0;
    }

    inline void AddFlag(U32 flag)
    {
        flags |= flag;
    }

    inline void RemoveFlag(U32 flag)
    {
        flags &= ~flag;
    }
};

struct GameState
{
    F32 virtualScreenWidth;
    const F32 virtualScreenHeight { 360.f };

    U64 tick;

    Camera camera;

    U64 nextEntityId { 1 };
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
    void Update(F32 deltaTime);
    void DrawFrame(F32 deltaTime);
    void UpdateAndDrawFrame(F32 deltaTime);
}