#include "game.h"

#include "input/input.h"

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <cassert>

namespace Game
{
    Vec2S32 TileCoordinateFromPoint(const Vec2F32 point)
    {
        Vec2S32 gridCoordinate;
        gridCoordinate.x = static_cast<S32>(std::floorf(point.x / g_TileSize));
        gridCoordinate.y = static_cast<S32>(std::floorf(point.y / g_TileSize));
        return gridCoordinate;
    }

    Vec2F32 VirtualPositonFromScreenPoint(const Vec2F32 point)
    {
        Vec2F32 virtualPosition;
        RectF32 screenRect { Renderer::GetScreenRect() };
        virtualPosition.x = point.x * (gameState.virtualScreenWidth / screenRect.width);
        virtualPosition.y = point.y * (gameState.virtualScreenHeight / screenRect.height);
        return virtualPosition;
    }

    bool CheckCollisionPointInRect(Vec2F32 point, const RectF32& rectangle)
    {
        return (point.x > rectangle.x && point.x < rectangle.x + rectangle.width
            && point.y > rectangle.y && point.y < rectangle.y + rectangle.height);
    }

    Vec2F32 GetMouseVirtualPositon()
    {
        return VirtualPositonFromScreenPoint(Input::GetMousePosition());
    }

    Entity* GolemFromWorldPosition(const Vec2F32 position)
    {
        for (auto& entity : gameState.entities)
        {
            if (entity.kind != EntityKind::Golem) continue;
            size_t idx = entity.animationIdx;
            RectF32 entityRect { entity.position.x, entity.position.y, static_cast<F32>(entity.animations[idx].frameWidth), static_cast<F32>(entity.animations[idx].frameHeight) };
            if (CheckCollisionPointInRect(position, entityRect))
            {
                return &entity;
            }
        }

        return nullptr;
    }

    Entity* CropFromWorldPosition(const Vec2F32 position)
    {
        for (auto& entity : gameState.entities)
        {
            if (entity.kind != EntityKind::Crop) continue;
            size_t idx { entity.animationIdx };
            RectF32 entityRect { entity.position.x, entity.position.y, static_cast<F32>(entity.animations[idx].frameWidth), static_cast<F32>(entity.animations[idx].frameHeight) };
            if (CheckCollisionPointInRect(position, entityRect))
            {
                return &entity;
            }
        }

        return nullptr;
    }

    Entity* CauldronFromWorldPosition(const Vec2F32 position)
    {
        for (auto& entity : gameState.entities)
        {
            if (entity.kind != EntityKind::Cauldron) continue;
            size_t idx = entity.animationIdx;
            RectF32 entityRect { entity.position.x, entity.position.y, static_cast<F32>(entity.animations[idx].frameWidth), static_cast<F32>(entity.animations[idx].frameHeight) };
            if (CheckCollisionPointInRect(position, entityRect))
            {
                return &entity;
            }
        }

        return nullptr;
    }

    Entity* EntityFromHandle(const EntityHandle handle)
    {
        if (handle.id == 0)
        {
            return nullptr;
        }
        if (handle.index >= gameState.entities.size())
        {
            return nullptr;
        }

        Entity* entity { &gameState.entities[handle.index] };
        if (entity->handle.id == handle.id)
        {
            return entity;
        }

        return nullptr;
    }

    bool CheckCollidableFromTile(const Vec2S32 position)
    {
        for (const auto& entity : gameState.entities)
        {
            if (!entity.HasFlag(EntityFlags::Collidable)) continue;

            Vec2S32 entityTile { TileCoordinateFromPoint(entity.position) };


            if (position.x >= entityTile.x && position.x < entityTile.x + entity.collisionWidth &&
                position.y >= entityTile.y && position.y < entityTile.y + entity.collisionHeight)
            {
                return true;
            }
        }
        return false;
    }

    // TODO:: Ad-hoc tilemap loading
    void LoadTileMap(std::filesystem::path path)
    {
        std::ifstream file;
        file.open(path);

        S32 x { 0 };
        S32 y { 0 };

        std::string line;

        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string token;

            while (std::getline(ss, token, ','))
            {
                Tile tile;
                tile.kind = TileKind::None;
                switch (std::stoi(token))
                {
                case -1:                                   break;
                case 1: tile.kind = TileKind::Grass_1;     break;
                case 2: tile.kind = TileKind::Grass_2;     break;
                case 3: tile.kind = TileKind::Grass_3;     break;
                }

                if (x < g_TileMapWidth && y < g_TileMapHeight)
                {
                    g_TileMap[x][y] = tile;
                }
                x++;
            }
            x = 0;
            y++;
        }

        file.close();
    }

    std::vector<Vec2S32> FindPath(Vec2S32 start, Vec2S32 target)
    {
        struct Node
        {
            S32 cost;
            S32 heuristic;
            Vec2S32 position;

            bool operator>(const Node& other) const
            {
                if (cost == other.cost)
                {
                    return heuristic > other.heuristic;
                }
                return cost > other.cost;
            }
        };

        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> frontier;
        frontier.push({ 0, 0, {start.x, start.y} });

        bool reached[g_TileMapWidth][g_TileMapHeight] = {};
        reached[start.x][start.y] = true;

        Vec2S32 cameFrom[g_TileMapWidth][g_TileMapHeight] = {};

        S32 costSoFar[g_TileMapWidth][g_TileMapHeight] = {};
        costSoFar[start.x][start.y] = 0;

        auto Heuristic { [](Vec2S32 start, Vec2S32 target) -> S32
            {
                Vec2S32 delta { std::abs(start.x - target.x), std::abs(start.y - target.y) };
                return 10 * (delta.x + delta.y) + (-6) * std::min(delta.x, delta.y);
            } };

        while (!frontier.empty())
        {
            Node current { frontier.top() };
            frontier.pop();

            if (current.position.x == target.x &&
                current.position.y == target.y)
            {
                std::vector<Vec2S32> path;
                Vec2S32 step { target };

                while (!(step.x == start.x && step.y == start.y))
                {
                    path.push_back(step);
                    step = cameFrom[step.x][step.y];

                    if (!reached[step.x][step.y]) break;
                }
                return path;
            }

            std::vector<Vec2S32> delta { {0, -1}, {1, -1,}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1} };

            for (S32 i { 0 }; i < 8; i++)
            {
                S32 neighbourX { current.position.x + delta[i].x };
                S32 neighbourY { current.position.y + delta[i].y };

                if (neighbourX < 0 || neighbourX >= g_TileMapWidth ||
                    neighbourY < 0 || neighbourY >= g_TileMapHeight)
                {
                    continue;
                }

                if (CheckCollidableFromTile({ neighbourX, neighbourY }))
                {
                    continue;
                }

                S32 moveCost { (delta[i].x && delta[i].y) ? 14 : 10 };
                S32 newCost { costSoFar[current.position.x][current.position.y] + moveCost };

                if (!reached[neighbourX][neighbourY] || newCost < costSoFar[neighbourX][neighbourY])
                {
                    costSoFar[neighbourX][neighbourY] = newCost;
                    S32 heuristic { Heuristic({ neighbourX, neighbourY }, target) };
                    S32 cost { newCost + heuristic };
                    frontier.push({ cost, heuristic, {neighbourX, neighbourY} });
                    reached[neighbourX][neighbourY] = true;
                    cameFrom[neighbourX][neighbourY] = current.position;
                }
            }
        }

        return{};
    }

    Entity* CreateEntity(EntityKind kind);

    void SetupGolem(Entity& entity)
    {
        entity.texture = gameState.golemTexture;
        entity.speed = 3.0f;

        SpriteAnimation idle {};
        idle.currentFrame = 0;
        idle.frameCount = 2;
        idle.frameWidth = 16;
        idle.frameHeight = 16;
        idle.xOffset = 16;
        idle.yOffset = 16;
        idle.frameAdvancement = 30;
        SpriteAnimation carrying {};
        carrying.currentFrame = 0;
        carrying.frameCount = 2;
        carrying.frameWidth = 16;
        carrying.frameHeight = 16;
        carrying.xOffset = 16;
        carrying.yOffset = 48;
        carrying.frameAdvancement = 30;

        entity.animationIdx = 0;
        entity.animations[0] = idle;
        entity.animations[1] = carrying;
        entity.animationTicks = 0;

        auto GolemUpdate { [](Entity& entity, float deltaTime)
            {
                // Animation tick
                {
                    size_t idx { entity.animationIdx };
                    entity.animationTicks++;
                    U64 frame { (entity.animationTicks / entity.animations[idx].frameAdvancement) % entity.animations[idx].frameCount };
                    entity.animations[idx].currentFrame = static_cast<U32>(frame);
                }

                switch (entity.golemState)
                {
                case GolemState::Pathing:
                {
                    if (!entity.path.empty())
                    {
                        Vec2S32 nextTile { entity.path.back() };
                        Vec2F32 targetPosition { static_cast<F32>(nextTile.x) * g_TileSize, static_cast<F32>(nextTile.y) * g_TileSize };

                        F32 speed { entity.speed * g_TileSize };
                        F32 step { speed * deltaTime };

                        Vec2F32 delta { targetPosition - entity.position };
                        F32 distance { std::sqrt(delta.x * delta.x + delta.y * delta.y) };

                        if (distance <= step)
                        {
                            entity.position = targetPosition;
                            entity.path.pop_back();

                            if (entity.path.empty())
                            {
                                bool stateChanged { false };
                                Entity* crop { EntityFromHandle(entity.cropTargetHandle) };
                                if (crop)
                                {
                                    if (TileCoordinateFromPoint(crop->position) == TileCoordinateFromPoint(entity.position))
                                    {
                                        if (entity.heldItem.id == 0 && crop->harvestable && crop->kind == EntityKind::Crop)
                                        {
                                            entity.golemState = GolemState::Harvesting;
                                            stateChanged = true;
                                        }
                                    }
                                }

                                if (!stateChanged && entity.heldItem.id != 0)
                                {
                                    // Check if the golem is next to a cauldron to deposit the item
                                    for (const auto& cauldron : gameState.entities)
                                    {
                                        if (cauldron.kind != EntityKind::Cauldron) continue;
                                        Vec2S32 cauldronTile { TileCoordinateFromPoint(cauldron.position) };
                                        Vec2S32 golemTile { TileCoordinateFromPoint(entity.position) };
                                        if (golemTile.x >= cauldronTile.x - 1 && golemTile.x <= cauldronTile.x + 2 &&
                                            golemTile.y >= cauldronTile.y - 1 && golemTile.y <= cauldronTile.y + 2)
                                        {
                                            entity.golemState = GolemState::Depositing;
                                            stateChanged = true;
                                            break;
                                        }
                                    }
                                }

                                if (!stateChanged)
                                {
                                    entity.golemState = GolemState::Idle;
                                }
                            }
                        }
                        else
                        {
                            entity.position.x += (delta.x / distance) * step;
                            entity.position.y += (delta.y / distance) * step;
                        }
                    }
                    else
                    {
                        // Snap entity to tile grid
                        Vec2S32 entityTilePosition { TileCoordinateFromPoint(entity.position) };
                        Vec2F32 targetPosition { static_cast<F32>(entityTilePosition.x) * g_TileSize, static_cast<F32>(entityTilePosition.y) * g_TileSize };

                        F32 speed { entity.speed * g_TileSize };
                        F32 step { speed * deltaTime };

                        Vec2F32 delta { targetPosition - entity.position };
                        F32 distance { std::sqrt(delta.x * delta.x + delta.y * delta.y) };

                        if (distance <= step)
                        {
                            entity.position = targetPosition;
                        }
                        else
                        {
                            entity.position.x += (delta.x / distance) * step;
                            entity.position.y += (delta.y / distance) * step;
                        }
                    }
                }
                break;
                case GolemState::Harvesting:
                {
                    Entity* crop { EntityFromHandle(entity.cropTargetHandle) };
                    if (crop && crop->harvestable && crop->kind == EntityKind::Crop)
                    {
                        entity.animationIdx = 1;
                        entity.golemState = GolemState::Idle;
                        gameState.entities[crop->handle.index] = {};

                        Entity* daisyItem { CreateEntity(EntityKind::Item) };
                        if (daisyItem)
                        {
                            entity.heldItem = daisyItem->handle;
                        }
                    }
                }
                break;
                case GolemState::Depositing:
                {
                    Entity* item { EntityFromHandle(entity.heldItem) };
                    if (item)
                    {
                        gameState.entities[entity.heldItem.index] = {};
                    }
                    entity.heldItem = {};
                    entity.animationIdx = 0;
                    entity.golemState = GolemState::Idle;
                }
                break;
                }

                // update item positions
                if (entity.heldItem.id != 0)
                {
                    size_t idx { entity.animationIdx };

                    Entity* daisyItem { EntityFromHandle(entity.heldItem) };
                    F32 animationBob { entity.animations[idx].currentFrame ? 2.0f : 0.0f };
                    F32 offsetX { static_cast<F32>(daisyItem->texture.width) / 2.0f };
                    F32 offsetY { -(static_cast<F32>(daisyItem->texture.height) - 1.0f) + animationBob };

                    daisyItem->position.x = entity.position.x + offsetX;
                    daisyItem->position.y = entity.position.y + offsetY;
                }
            } };

        entity.update = GolemUpdate;
    }

    void SetupCrop(Entity& entity)
    {
        entity.texture = gameState.daisyTexture;
        entity.growthTicks = 0;

        SpriteAnimation daisyGrowth {};
        daisyGrowth.currentFrame = 0;
        daisyGrowth.frameCount = 4;
        daisyGrowth.frameWidth = 16;
        daisyGrowth.frameHeight = 16;
        daisyGrowth.xOffset = 16;
        daisyGrowth.yOffset = 16;

        entity.animations[entity.animationIdx] = daisyGrowth;
        entity.animationTicks = 0;

        auto CropUpdate { [](Entity& entity, float deltaTime)
            {
                if (entity.growthTicks < 1200)
                {
                    // Animation tick
                    {
                        size_t idx { entity.animationIdx };
                        entity.growthTicks++;
                        S32 stage { static_cast<S32>(entity.growthTicks / 300) };
                        entity.animations[idx].currentFrame = std::clamp(stage, 0, 3);
                        if (stage >= 3)
                        {
                            entity.harvestable = true;
                        }
                    }
                }
            } };

        entity.update = CropUpdate;
    }

    void SetupCauldron(Entity& entity)
    {
        entity.texture = gameState.cauldronTexture;
        entity.AddFlag(EntityFlags::Collidable);
        entity.collisionWidth = 2;
        entity.collisionHeight = 2;

        SpriteAnimation cauldronAnimation {};
        cauldronAnimation.currentFrame = 0;
        cauldronAnimation.frameCount = 7;
        cauldronAnimation.frameWidth = 32;
        cauldronAnimation.frameHeight = 32;
        cauldronAnimation.xOffset = 16;
        cauldronAnimation.yOffset = 16;
        cauldronAnimation.frameAdvancement = 8;

        entity.animations[entity.animationIdx] = cauldronAnimation;
        entity.animationTicks = 0;

        auto CauldronUpdate { [](Entity& entity, float deltaTime)
            {
                // Animation tick
                {
                    size_t idx { entity.animationIdx };
                    entity.animationTicks++;
                    U64 frame { (entity.animationTicks / entity.animations[idx].frameAdvancement) % entity.animations[idx].frameCount };
                    entity.animations[idx].currentFrame = static_cast<U32>(frame);
                }
            } };

        entity.update = CauldronUpdate;
    }
    
    void SetupItem(Entity& entity)
    {
        entity.texture = gameState.daisyItemTexture;

        SpriteAnimation itemAnimation {};
        itemAnimation.frameCount = 1;
        itemAnimation.frameWidth = entity.texture.width;
        itemAnimation.frameHeight = entity.texture.height;

        entity.animations[0] = itemAnimation;

        entity.update = [](Entity&, float) {};
    }

    Entity* CreateEntity(EntityKind kind)
    {
        Entity* newEntity { nullptr };
        for (size_t index = 0; index < gameState.entities.size(); ++index)
        {
            if (gameState.entities[index].handle.id == 0)
            {
                newEntity = &gameState.entities[index];
                *newEntity = {};
                newEntity->handle.id = gameState.nextEntityId++;
                newEntity->handle.index = index;
                newEntity->kind = kind;
                break;
            }
        }

        if (!newEntity)
        {
            assert(false && "Entity array maxxed out!");
            static Entity nullEntity {};
            nullEntity = {};
            return &nullEntity;
        }

        switch (kind)
        {
        case EntityKind::Golem:
        {
            SetupGolem(*newEntity);
        }
        break;
        case EntityKind::Crop:
        {
            SetupCrop(*newEntity);
        }
        break;
        case EntityKind::Cauldron:
        {
            SetupCauldron(*newEntity);
        }
        break;
        case EntityKind::Item:
        {
            SetupItem(*newEntity);
        }
        break;
        }

        return newEntity;
    }

    void Init()
    {
        Renderer::WindowCreate(1280, 720, L"Farming Sim Prototype");

        gameState.interactableTexture = Renderer::LoadTexture("data/textures/interactable.png");
        gameState.daisyItemTexture = Renderer::LoadTexture("data/textures/daisy_item.png");
        gameState.cauldronTexture = Renderer::LoadTexture("data/textures/cauldron.png");
        gameState.golemTexture = Renderer::LoadTexture("data/textures/golem.png");
        gameState.daisyTexture = Renderer::LoadTexture("data/textures/daisy.png");
        g_TileTextures[static_cast<size_t>(TileKind::Grass_1)] = Renderer::LoadTexture("data/textures/grass1.png");
        g_TileTextures[static_cast<size_t>(TileKind::Grass_2)] = Renderer::LoadTexture("data/textures/grass2.png");
        g_TileTextures[static_cast<size_t>(TileKind::Grass_3)] = Renderer::LoadTexture("data/textures/grass3.png");

        LoadTileMap("data/tilemap/tilemap1.csv");

        gameState.font1 = Renderer::LoadFont("data/font/romulus.ttf", 16.0f);
        gameState.font2 = Renderer::LoadFont("data/font/tiny5.ttf", 8.0f);

        Entity* cauldron { CreateEntity(EntityKind::Cauldron) };
        if (cauldron)
        {
            cauldron->position = { 80.0f * g_TileSize, 44.0f * g_TileSize };
        }

        Entity* golem { CreateEntity(EntityKind::Golem) };
        if (golem)
        {
            golem->position = { 80.0f * g_TileSize, 47.0f * g_TileSize }; // Near the cauldron in the center of the map
            golem->targetPosition = golem->position;
        }

        gameState.camera.position = { (g_TileMapWidth * g_TileSize) * 0.5f, (g_TileMapHeight * g_TileSize) * 0.5f };
        gameState.camera.offset = { 640 * 0.5f, 360 * 0.5f };
        gameState.camera.zoom = 1.0f;


        const std::vector<Vec2S32> daisyLocations {
            // Left field
            {67, 45}, {65, 46}, {63, 47}, {65, 47}, {66, 47},
            {62, 48}, {63, 48}, {65, 48}, {66, 48}, {67, 48},
            {64, 49}, {65, 49}, {66, 49}, {66, 50}, {69, 50},
            {62, 52}, {67, 52}, {66, 53},
            // Right field
            {96, 41}, {97, 42}, {98, 44}, {99, 44}, {98, 45},
            {99, 45}, {97, 46}, {98, 46}, {99, 46}, {96, 47},
            {98, 47}, {99, 47}, {100, 48}, {103, 49}, {101, 50}
        };

        for (const Vec2S32& location : daisyLocations)
        {
            Entity* daisy { CreateEntity(EntityKind::Crop) };
            if (daisy)
            {
                daisy->position = { location.x * static_cast<F32>(g_TileSize), location.y * static_cast<F32>(g_TileSize) };
            }
        }
    }

    void Update(F32 deltaTime)
    {
        Input::ProcessEvents();

        
        if (Input::IsKeyDown(Key::W))
        {
            gameState.camera.position.y -= 5.0f / gameState.camera.zoom;
        }
        if (Input::IsKeyDown(Key::A))
        {
            gameState.camera.position.x -= 5.0f / gameState.camera.zoom;
        }
        if (Input::IsKeyDown(Key::S))
        {
            gameState.camera.position.y += 5.0f / gameState.camera.zoom;
        }
        if (Input::IsKeyDown(Key::D))
        {
            gameState.camera.position.x += 5.0f / gameState.camera.zoom;
        }

        if (Input::IsKeyPressed(Key::G))
        {
            Vec2F32 virtualMousePosition { GetMouseVirtualPositon() };
            Vec2F32 virtualWorldPosition { WorldFromScreen(virtualMousePosition, gameState.camera) };
            Vec2S32 gridPosition { TileCoordinateFromPoint(virtualWorldPosition) };
            
            if (gridPosition.x >= 0 && gridPosition.x < g_TileMapWidth && gridPosition.y >= 0 && gridPosition.y < g_TileMapHeight && !CheckCollidableFromTile(gridPosition))
            {
                Entity* golem { CreateEntity(EntityKind::Golem) };
                if (golem)
                {
                    golem->position = { static_cast<F32>(gridPosition.x) * g_TileSize, static_cast<F32>(gridPosition.y) * g_TileSize };
                    golem->targetPosition = golem->position;
                }
            }
        }

        if (Input::IsKeyPressed(Key::ESCAPE))
        {
            gameState.activeEntityHandle.id = 0;
        }

        RectF32 screenRect { Renderer::GetScreenRect() };
        F32 aspectRatio { screenRect.width / screenRect.height };
        gameState.virtualScreenWidth =  gameState.virtualScreenHeight * aspectRatio;
        gameState.camera.offset = { gameState.virtualScreenWidth * 0.5f, gameState.virtualScreenHeight * 0.5f };

        F32 scrollWheelDelta { Input::GetScrollDelta() };
        if (scrollWheelDelta != 0.0f)
        {
            Vec2F32 virtualMousePosition { GetMouseVirtualPositon() };

            Vec2F32 previousWorldPosition { WorldFromScreen(virtualMousePosition, gameState.camera) };

            // TODO:: Figure out what amount of camera steps I want and how far each step will be,
            // linear steps should be the best for this type of game.
            gameState.camera.zoom += scrollWheelDelta / 10.0f;
            gameState.camera.zoom = std::clamp(gameState.camera.zoom, 0.5f, 1.5f);

            Vec2F32 postZoomWorldPos { WorldFromScreen(virtualMousePosition, gameState.camera) };

            gameState.camera.position.x += (previousWorldPosition.x - postZoomWorldPos.x);
            gameState.camera.position.y += (previousWorldPosition.y - postZoomWorldPos.y);
        }

        // Pick the entity
        if (Input::IsKeyPressed(Key::MOUSE_LEFT))
        {
            Vec2F32 virtualMousePosition { GetMouseVirtualPositon() };
            Vec2F32 virtualWorldPosition { WorldFromScreen(virtualMousePosition, gameState.camera) };

            Entity* golem { GolemFromWorldPosition(virtualWorldPosition) };
            if (golem)
            {
                gameState.activeEntityHandle = golem->handle;
            }
        }

        // Click to move the picked entity and provide it with a path
        if (Input::IsKeyPressed(Key::MOUSE_RIGHT))
        {
            Entity* entity { EntityFromHandle(gameState.activeEntityHandle) };
            if (entity && entity->kind == EntityKind::Golem)
            {
                Vec2F32 virtualMousePosition { GetMouseVirtualPositon() };
                Vec2F32 virtualWorldPosition { WorldFromScreen(virtualMousePosition, gameState.camera) };

                if (entity->golemState == GolemState::Idle ||
                    entity->golemState == GolemState::Pathing)
                {
                    Entity* crop { CropFromWorldPosition(virtualWorldPosition) };
                    Entity* cauldron { CauldronFromWorldPosition(virtualWorldPosition) };

                    if (crop || !cauldron)
                    {
                        Vec2S32 startPosition { TileCoordinateFromPoint(entity->position) };
                        Vec2S32 targetPosition { TileCoordinateFromPoint(virtualWorldPosition) };

                        if (crop)
                        {
                            entity->cropTargetHandle = crop->handle;
                        }
                        else
                        {
                            entity->cropTargetHandle.id = 0;
                            entity->cropTargetHandle.index = 0;
                        }
                        entity->path = FindPath(startPosition, targetPosition);

                        if (!entity->path.empty())
                        {
                            entity->golemState = GolemState::Pathing;
                        }
                    }
                }

                if (entity->heldItem.id != 0)
                {
                    Entity* cauldron { CauldronFromWorldPosition(virtualWorldPosition) };

                    if (cauldron)
                    {
                        Vec2S32 cauldronTile { TileCoordinateFromPoint(cauldron->position) };
                        Vec2S32 entityTile { TileCoordinateFromPoint(entity->position) };

                        auto IsCauldronTile { [&cauldronTile](Vec2S32 tile) -> bool
                            {
                                return tile.x >= cauldronTile.x && tile.x <= cauldronTile.x + 1 &&
                                    tile.y >= cauldronTile.y && tile.y <= cauldronTile.y + 1;
                            } };

                        Vec2S32 bestTile {};
                        F32 bestDistance { std::numeric_limits<F32>::max() };

                        for (S32 x = cauldronTile.x - 1; x <= cauldronTile.x + 2; x++)
                        {
                            for (S32 y = cauldronTile.y - 1; y <= cauldronTile.y + 2; y++)
                            {
                                Vec2S32 candidate { x, y };

                                if (IsCauldronTile(candidate)) continue;

                                F32 dx { static_cast<F32>(entityTile.x - x) };
                                F32 dy { static_cast<F32>(entityTile.y - y) };
                                F32 distSq { (dx * dx) + (dy * dy) };

                                if (distSq < bestDistance)
                                {
                                    bestDistance = distSq;
                                    bestTile = candidate;
                                }
                            }
                        }

                        Vec2S32 startPosition { TileCoordinateFromPoint(entity->position) };
                        Vec2S32 targetPosition { bestTile };
                        entity->path = FindPath(startPosition, targetPosition);

                        if (!entity->path.empty())
                        {
                            entity->golemState = GolemState::Pathing;
                        }
                    }

                    
                }
            }
        }

        for (auto& entity : gameState.entities)
        {
            if (entity.kind == EntityKind::None) continue;
            entity.update(entity, deltaTime);
        }
    }

    void DrawFrame(F32 deltaTime)
    {
        Renderer::BeginFrame(gameState.virtualScreenWidth, gameState.virtualScreenHeight);

            Renderer::BeginModeWorldSpace(gameState.camera);

                Vec2F32 minWorld { WorldFromScreen({ 0, 0 }, gameState.camera) };
                Vec2F32 maxWorld { WorldFromScreen({ gameState.virtualScreenWidth, gameState.virtualScreenHeight }, gameState.camera) };

                static constexpr F32 invTileSize { 1.0f / g_TileSize };
                S32 startX { std::clamp(static_cast<S32>(floorf(minWorld.x * invTileSize)), 0, g_TileMapWidth) };
                S32 startY { std::clamp(static_cast<S32>(floorf(minWorld.y * invTileSize)), 0, g_TileMapHeight) };
                S32 endX { std::clamp(static_cast<S32>(ceilf(maxWorld.x * invTileSize)), 0, g_TileMapWidth) };
                S32 endY { std::clamp(static_cast<S32>(ceilf(maxWorld.y * invTileSize)), 0, g_TileMapHeight) };

                for (S32 y = startY; y < endY; y++)
                {
                    for (S32 x = startX; x < endX; x++)
                    {
                        Tile tile { g_TileMap[x][y] };
                        if (tile.kind == TileKind::None)
                        {
                            continue;
                        }
                        RectF32 dest { static_cast<F32>(x * g_TileSize), static_cast<F32>(y * g_TileSize), g_TileSize, g_TileSize };
                        Renderer::DrawSprite(g_TileTextures[static_cast<size_t>(tile.kind)], {dest.x, dest.y, 100.0f}, dest.width, dest.height);
                    }
                }

                for (const auto& entity : gameState.entities)
                {
                    if (entity.kind == EntityKind::None) continue;

                    F32 entityMinX { entity.position.x };
                    F32 entityMinY { entity.position.y };
                    F32 entityMaxX { entity.position.x + static_cast<F32>(entity.texture.width) };
                    F32 entityMaxY { entity.position.y + static_cast<F32>(entity.texture.height) };

                    bool visible { entityMaxX >= minWorld.x && entityMinX <= maxWorld.x &&
                                   entityMaxY >= minWorld.y && entityMinY <= maxWorld.y };

                    if (visible)
                    {
                        entity.draw(entity);
                    }
                }

                // Draw active entity overlay
                {
                    Entity* entity { EntityFromHandle(gameState.activeEntityHandle) };
                    if (entity)
                    {
                        RectF32 dest;
                        dest.width = gameState.interactableTexture.width;
                        dest.height = gameState.interactableTexture.height;
                        dest.x = entity->position.x;
                        dest.y = entity->position.y;

                        F32 entityMinX { dest.x };
                        F32 entityMinY { dest.y };
                        F32 entityMaxX { dest.x + static_cast<F32>(dest.width) };
                        F32 entityMaxY { dest.y + static_cast<F32>(dest.height) };

                        bool visible { entityMaxX >= minWorld.x && entityMinX <= maxWorld.x &&
                                       entityMaxY >= minWorld.y && entityMinY <= maxWorld.y };

                        if (visible)
                        {
                            Renderer::DrawSprite(gameState.interactableTexture, { dest.x, dest.y, 0.0f }, dest.width, dest.height);
                        }
                    }
                }

            Renderer::EndMode();

            Renderer::BeginModeScreenSpace();

                Renderer::DrawText(gameState.font1, "FPS: " + std::to_string(Renderer::GetFPS()), 5, 5, gameState.font1.size);
                Renderer::DrawText(gameState.font1, "Draw Call Count: " + std::to_string(Renderer::GetDrawCallCount()), 5, 15, gameState.font1.size);

                {
                    Vec2F32 virtualMousePosition { GetMouseVirtualPositon() };
                    Vec2F32 virtualWorldPosition { WorldFromScreen(virtualMousePosition, gameState.camera) };
                    Vec2S32 gridPosition { TileCoordinateFromPoint(virtualWorldPosition) };
                    Renderer::DrawText(gameState.font1, "Mouse Grid Position: " + std::to_string(gridPosition.x) + ", " + std::to_string(gridPosition.y), 5, 25, gameState.font1.size);
                }

                // Debug hover text
                {
                    Vec2F32 virtualMousePosition { GetMouseVirtualPositon() };
                    Vec2F32 virtualWorldPosition { WorldFromScreen(virtualMousePosition, gameState.camera) };
                    Entity* golem { CauldronFromWorldPosition(virtualWorldPosition) };

                    if (golem)
                    {
                        size_t idx { golem->animationIdx };
                        RectF32 entityRect { golem->position.x, golem->position.y, static_cast<F32>(golem->animations[idx].frameWidth), static_cast<F32>(golem->animations[idx].frameHeight) };
                        if (CheckCollisionPointInRect(virtualWorldPosition, entityRect))
                        {
                            Renderer::DrawText(gameState.font1, "Hovered", virtualMousePosition.x, virtualMousePosition.y, gameState.font1.size);
                        }
                    }
                }

            Renderer::EndMode();

        Renderer::EndFrame();
    }

    void UpdateAndDrawFrame(F32 deltaTime)
    {
        gameState.tick++;

        Update(deltaTime);
        DrawFrame(deltaTime);
    }
}