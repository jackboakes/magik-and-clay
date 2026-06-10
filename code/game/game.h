#pragma once

#include "camera/camera.h"
#include "renderer/renderer.h"
#include "common/types.h"

#include <filesystem>
#include <array>

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
    Collidable = 1 << 0,
    Harvestable = 1 << 1 
};

struct SpriteAnimation
{
    U32 frameCount { 1 };
    U32 currentFrame { 0 };
    U32 frameWidth { 0 };
    U32 frameHeight { 0 };
    U32 xOffset { 0 };
    U32 yOffset { 0 };
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

    void (*update)(Entity&, float dt);
    void (*draw)(const Entity&) { [](const Entity& entity) 
        {
            size_t idx { entity.animationIdx };
            U32 height { entity.animations[idx].frameHeight };
            U32 width { entity.animations[idx].frameWidth };

            RectF32 source { 0 };
            source.width = width;
            source.height = height;
            source.x = entity.animations[idx].xOffset + (entity.animations[idx].currentFrame * (entity.animations[idx].xOffset + width));
            source.y = entity.animations[idx].yOffset;

            RectF32 destination { 0 };
            destination.width = width;
            destination.height = height;
            destination.x = entity.position.x;
            destination.y = entity.position.y;

            Renderer::DrawSprite(entity.texture, { destination.x, destination.y, 1.0f - (entity.position.y / (g_TileMapHeight * g_TileSize)) }, destination.width, destination.height, source);
        } };

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
    Texture daisyTexture;
    Texture daisyItemTexture;
    Texture cauldronTexture;
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