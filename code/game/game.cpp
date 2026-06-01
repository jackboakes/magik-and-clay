#include "game.h"

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>

#include "win32/win_core.h" // TODO:: remove this depenedency on win32 layer

#include "input/input.h"

#include "renderer/backend/d3d11_backend.h" // For temporary debug code to get sprite batch count


namespace Game
{
    Vec2S32 TileCoordinateFromPoint(const Vec2F32 point)
    {
        Vec2S32 gridCoordinate;
        gridCoordinate.x = static_cast<int32_t>(std::floorf(point.x / g_TileSize));
        gridCoordinate.y = static_cast<int32_t>(std::floorf(point.y / g_TileSize));
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
            size_t idx = entity.animationIdx;
            RectF32 entityRect { entity.position.x, entity.position.y, static_cast<float>(entity.animations[idx].frameWidth), static_cast<float>(entity.animations[idx].frameHeight) };
            if (CheckCollisionPointInRect(position, entityRect))
            {
                if (entity.type == EntityType::Golem)
                {
                    return &entity;
                }
            }
        }

        return nullptr;
    }

    Entity* CropFromWorldPosition(const Vec2F32 position)
    {
        for (auto& entity : gameState.entities)
        {
            size_t idx = entity.animationIdx;
            RectF32 entityRect { entity.position.x, entity.position.y, static_cast<float>(entity.animations[idx].frameWidth), static_cast<float>(entity.animations[idx].frameHeight) };
            if (CheckCollisionPointInRect(position, entityRect))
            {
                if (entity.type == EntityType::Crop)
                {
                    return &entity;
                }
            }
        }

        return nullptr;
    }

    Entity* CauldronFromWorldPosition(const Vec2F32 position)
    {
        for (auto& entity : gameState.entities)
        {
            size_t idx = entity.animationIdx;
            RectF32 entityRect { entity.position.x, entity.position.y, static_cast<float>(entity.animations[idx].frameWidth), static_cast<float>(entity.animations[idx].frameHeight) };
            if (CheckCollisionPointInRect(position, entityRect))
            {
                if (entity.type == EntityType::Cauldron)
                {
                    return &entity;
                }
            }
        }

        return nullptr;
    }

    Entity* EntityFromId(const EntityId id)
    {
        for (auto& entity : gameState.entities)
        {
            if (entity.id == id)
            {
                return &entity;
            }
        }
        return nullptr;
    }

    bool CheckCollidableFromTile(const Vec2S32 position)
    {
        for (const auto& entity : gameState.entities)
        {
            if (!entity.collidable) continue;

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

        int x { 0 };
        int y { 0 };

        std::string line;

        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string token;

            while (std::getline(ss, token, ','))
            {
                Tile tile;
                tile.type = TileType::None;
                switch (std::stoi(token))
                {
                case -1:                                   break;
                case 1: tile.type = TileType::Grass_1;     break;
                case 2: tile.type = TileType::Grass_2;     break;
                case 3: tile.type = TileType::Grass_3;     break;
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
            int cost;
            int heuristic;
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

        int costSoFar[g_TileMapWidth][g_TileMapHeight] = {};
        costSoFar[start.x][start.y] = 0;

        auto Heuristic = [](Vec2S32 start, Vec2S32 target)
            {
                Vec2S32 delta { std::abs(start.x - target.x), std::abs(start.y - target.y) };
                return 10 * (delta.x + delta.y) + (-6) * std::min(delta.x, delta.y);
            };

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

            for (int i { 0 }; i < 8; i++)
            {
                int neighbourX { current.position.x + delta[i].x };
                int neighbourY { current.position.y + delta[i].y };

                if (neighbourX < 0 || neighbourX >= g_TileMapWidth ||
                    neighbourY < 0 || neighbourY >= g_TileMapHeight)
                {
                    continue;
                }

                if (CheckCollidableFromTile({ neighbourX, neighbourY }))
                {
                    continue;
                }

                int moveCost { (delta[i].x && delta[i].y) ? 14 : 10 };
                int newCost { costSoFar[current.position.x][current.position.y] + moveCost };

                if (!reached[neighbourX][neighbourY] || newCost < costSoFar[neighbourX][neighbourY])
                {
                    costSoFar[neighbourX][neighbourY] = newCost;
                    int heuristic { Heuristic({ neighbourX, neighbourY }, target) };
                    int cost { newCost + heuristic };
                    frontier.push({ cost, heuristic, {neighbourX, neighbourY} });
                    reached[neighbourX][neighbourY] = true;
                    cameFrom[neighbourX][neighbourY] = current.position;
                }
            }
        }

        return{};
    }
    
    void Init()
    {
        Renderer::WindowCreate(1280, 720, L"Farming Sim Prototype");

        gameState.entities.reserve(256);

        gameState.interactableTexture = Renderer::LoadTexture("../data/textures/interactable.png");
        gameState.daisyItemTexture = Renderer::LoadTexture("../data/textures/daisy_item.png");
        Texture cauldronTexture { Renderer::LoadTexture("../data/textures/cauldron.png") };


        Entity cauldron {};
        cauldron.id = gameState.nextEntityId++;
        cauldron.type = EntityType::Cauldron;
        cauldron.texture = cauldronTexture;
        cauldron.position = { 80.0f * g_TileSize, 44.0f * g_TileSize };
        cauldron.collidable = true;
        cauldron.collisionWidth = 2;
        cauldron.collisionHeight = 2;

        SpriteAnimation cauldronAnimation {};
        cauldronAnimation.currentFrame = 0;
        cauldronAnimation.frameCount = 7;
        cauldronAnimation.frameWidth = 32;
        cauldronAnimation.frameHeight = 32;
        cauldronAnimation.xOffset = 16;
        cauldronAnimation.yOffset = 16;
        cauldronAnimation.frameAdvancement = 8;

        cauldron.animations[cauldron.animationIdx] = cauldronAnimation;
        gameState.entities.push_back(cauldron);

        gameState.golemTexture = Renderer::LoadTexture("../data/textures/golem.png");

        Entity golem {};
        golem.id = gameState.nextEntityId++;
        golem.type = EntityType::Golem;
        golem.texture = gameState.golemTexture;
        golem.position = { 80.0f * g_TileSize, 47.0f * g_TileSize }; // Near the cauldron in the center of the map
        golem.targetPosition = golem.position;
        golem.speed = 3.0f;

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

        golem.animationIdx = 0;
        golem.animations[static_cast<int>(GolemState::Idle)] = idle;
        golem.animations[0] = idle; // carrying animation is tied to bool and not state
        golem.animations[1] = carrying;

        gameState.entities.push_back(golem);

        g_TileTextures[static_cast<size_t>(TileType::Grass_1)] = Renderer::LoadTexture("../data/textures/grass1.png");
        g_TileTextures[static_cast<size_t>(TileType::Grass_2)] = Renderer::LoadTexture("../data/textures/grass2.png");
        g_TileTextures[static_cast<size_t>(TileType::Grass_3)] = Renderer::LoadTexture("../data/textures/grass3.png");

        LoadTileMap("../data/tilemap/tilemap1.csv");

        gameState.font1 = Renderer::LoadFont("../data/font/romulus.ttf", 16.0f);
        gameState.font2 = Renderer::LoadFont("../data/font/tiny5.ttf", 8.0f);

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

        Texture daisyAtlas { Renderer::LoadTexture("../data/textures/daisy.png") };
        SpriteAnimation daisyGrowth {};
        daisyGrowth.currentFrame = 0;
        daisyGrowth.frameCount = 4;
        daisyGrowth.frameWidth = 16;
        daisyGrowth.frameHeight = 16;
        daisyGrowth.xOffset = 16;
        daisyGrowth.yOffset = 16;

        for (const Vec2S32& location : daisyLocations)
        {
            Entity daisy;
            daisy.id = gameState.nextEntityId++;
            daisy.type = EntityType::Crop;
            daisy.texture = daisyAtlas;
            daisy.position = { location.x * static_cast<float>(g_TileSize), location.y * static_cast<float>(g_TileSize) };
            daisy.animations[0] = daisyGrowth;
            daisy.animationTicks = 0;
            daisy.growthTicks = 0;
            gameState.entities.push_back(daisy);
        }
    }

    void Update(float deltaTime)
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
                Entity golem {};
                golem.id = gameState.nextEntityId++;
                golem.type = EntityType::Golem;
                golem.texture = gameState.golemTexture;
                golem.position = { static_cast<float>(gridPosition.x) * g_TileSize, static_cast<float>(gridPosition.y) * g_TileSize };
                golem.targetPosition = golem.position;
                golem.speed = 3.0f;
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

                golem.animations[static_cast<int>(GolemState::Idle)] = idle;
                golem.animations[0] = idle; // carrying animation is tied to bool and not state
                golem.animations[1] = carrying;

                gameState.entities.push_back(golem);
            }
        }

        if (Input::IsKeyPressed(Key::ESCAPE))
        {
            gameState.activeEntity = nullptr;
        }


        RectF32 screenRect { Renderer::GetScreenRect() };
        float aspectRatio { screenRect.width / screenRect.height };
        gameState.virtualScreenWidth =  gameState.virtualScreenHeight * aspectRatio;
        gameState.camera.offset = { gameState.virtualScreenWidth * 0.5f, gameState.virtualScreenHeight * 0.5f };

        float scrollWheelDelta { Input::GetScrollDelta() };
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

        for (auto& entity : gameState.entities)
        {
            if (entity.type != EntityType::Golem) continue;
            size_t idx { entity.animationIdx };
            entity.animationTicks++;
            uint64_t frame { (entity.animationTicks / entity.animations[idx].frameAdvancement) % entity.animations[idx].frameCount };
            entity.animations[idx].currentFrame = static_cast<uint32_t>(frame);
        }

        for (auto& entity : gameState.entities)
        {
            if (entity.type != EntityType::Cauldron) continue;
            size_t idx { entity.animationIdx };
            entity.animationTicks++;
            uint64_t frame { (entity.animationTicks / entity.animations[idx].frameAdvancement) % entity.animations[idx].frameCount };
            entity.animations[idx].currentFrame = static_cast<uint32_t>(frame);
        }

        for (auto& entity : gameState.entities)
        {
            if (entity.type != EntityType::Crop) continue;
            if (entity.growthTicks >= 1200) continue;
            size_t idx { entity.animationIdx };
            entity.growthTicks++;
            int stage { static_cast<int>(entity.growthTicks / 300) };
            entity.animations[idx].currentFrame = std::clamp(stage, 0, 3);
            if (stage >= 3)
            {
                entity.harvestable = true;
            }
        }

        // Pick the entity
        if (Input::IsKeyPressed(Key::MOUSE_LEFT))
        {
            Vec2F32 virtualMousePosition { GetMouseVirtualPositon() };
            Vec2F32 virtualWorldPosition { WorldFromScreen(virtualMousePosition, gameState.camera) };

            Entity* golem { GolemFromWorldPosition(virtualWorldPosition) };
            if (golem)
            {
                gameState.activeEntity = golem;
            }
        }

        // Click to move the picked entity and provide it with a path
        if (Input::IsKeyPressed(Key::MOUSE_RIGHT))
        {
            if (gameState.activeEntity && gameState.activeEntity->type == EntityType::Golem)
            {

                if (gameState.activeEntity->golemState == GolemState::Idle ||
                    gameState.activeEntity->golemState == GolemState::Pathing)
                {
                    Vec2F32 virtualMousePosition { GetMouseVirtualPositon() };
                    Vec2F32 virtualWorldPosition { WorldFromScreen(virtualMousePosition, gameState.camera) };
                    Entity* crop { CropFromWorldPosition(virtualWorldPosition) };

                    Vec2S32 startPosition { TileCoordinateFromPoint(gameState.activeEntity->position) };
                    Vec2S32 targetPosition { TileCoordinateFromPoint(virtualWorldPosition) };

                    if (crop)
                    {
                        gameState.activeEntity->cropTargetId = crop->id;
                    }
                    else
                    {
                        gameState.activeEntity->cropTargetId = 0;
                    }
                    gameState.activeEntity->path = FindPath(startPosition, targetPosition);

                    if (!gameState.activeEntity->path.empty())
                    {
                        gameState.activeEntity->golemState = GolemState::Pathing;
                    }
                }
            }
        }

        // Update entity movement
        {
            // Update the entities path
            for (auto& entity : gameState.entities)
            {
                if (entity.type != EntityType::Golem) continue;
                if (entity.path.empty()) continue;
                if (entity.golemState != GolemState::Pathing) continue;

                Vec2S32 nextTile { entity.path.back() };
                Vec2F32 targetPosition { static_cast<float>(nextTile.x) * g_TileSize, static_cast<float>(nextTile.y) * g_TileSize };

                float speed { entity.speed * g_TileSize };
                float step { speed * deltaTime };

                Vec2F32 delta { targetPosition - entity.position };
                float distance { std::sqrt(delta.x * delta.x + delta.y * delta.y) };

                if (distance <= step)
                {
                    entity.position = targetPosition;
                    entity.path.pop_back();

                    if (entity.path.empty())
                    {
                        Entity* crop { EntityFromId(entity.cropTargetId) };
                        if (crop)
                        {
                            if (TileCoordinateFromPoint(crop->position) == TileCoordinateFromPoint(entity.position))
                            {
                                if (entity.heldItem == 0 && crop->harvestable && crop->type == EntityType::Crop)
                                {
                                    entity.golemState = GolemState::Harvesting;
                                }
                            }
                        }
                    }
                }
                else
                {
                    entity.position.x += (delta.x / distance) * step;
                    entity.position.y += (delta.y / distance) * step;
                }
            }

            // Snap picked entity to tile grid
            if (gameState.activeEntity && gameState.activeEntity->path.empty())
            {
                Vec2S32 entityTilePosition { TileCoordinateFromPoint(gameState.activeEntity->position) };
                Vec2F32 targetPosition { static_cast<float>(entityTilePosition.x) * g_TileSize, static_cast<float>(entityTilePosition.y) * g_TileSize };

                float speed { gameState.activeEntity->speed * g_TileSize };
                float step { speed * deltaTime };

                Vec2F32 delta { targetPosition - gameState.activeEntity->position };
                float distance { std::sqrt(delta.x * delta.x + delta.y * delta.y) };

                if (distance <= step)
                {
                    gameState.activeEntity->position = targetPosition;
                }
                else
                {
                    gameState.activeEntity->position.x += (delta.x / distance) * step;
                    gameState.activeEntity->position.y += (delta.y / distance) * step;
                }
            }
        }

        for (auto& entity : gameState.entities)
        {
            if (entity.type != EntityType::Golem) continue;
            if (entity.golemState != GolemState::Harvesting) continue;

            Entity* crop { EntityFromId(entity.cropTargetId)};
            if (crop && crop->harvestable && crop->type == EntityType::Crop)
            {
                entity.animationIdx = 1;
                entity.golemState = GolemState::Idle;
                crop->markedForDeletion = true;

                Entity daisyItem {};
                daisyItem.id = gameState.nextEntityId++;
                daisyItem.type = EntityType::Item;
                daisyItem.texture = gameState.daisyItemTexture;
                gameState.entities.push_back(daisyItem);

                entity.heldItem = daisyItem.id;
            }
        }

        for (auto& entity : gameState.entities)
        {
            if (entity.heldItem == 0) continue;
            size_t idx { entity.animationIdx };
            

            Entity* daisyItem { EntityFromId(entity.heldItem) };
            float animationBob { entity.animations[idx].currentFrame ? 2.0f : 0.0f };
            float offsetX { static_cast<float>(daisyItem->texture.width) / 2.0f };
            float offsetY { -(static_cast<float>(daisyItem->texture.height) - 1.0f) + animationBob };

            daisyItem->position.x = entity.position.x + offsetX;
            daisyItem->position.y = entity.position.y + offsetY;
        }

        std::erase_if(gameState.entities, [](const Entity& entity)
        {
            return entity.markedForDeletion;
        });
    }

    void DrawFrame(float deltaTime)
    {
        Renderer::BeginFrame(gameState.virtualScreenWidth, gameState.virtualScreenHeight);

            Renderer::BeginModeWorldSpace(gameState.camera);

                Vec2F32 minWorld { WorldFromScreen({ 0, 0 }, gameState.camera) };
                Vec2F32 maxWorld { WorldFromScreen({ gameState.virtualScreenWidth, gameState.virtualScreenHeight }, gameState.camera) };

                static constexpr float invTileSize { 1.0f / g_TileSize };
                int startX { std::clamp(static_cast<int>(floorf(minWorld.x * invTileSize)), 0, g_TileMapWidth) };
                int startY { std::clamp(static_cast<int>(floorf(minWorld.y * invTileSize)), 0, g_TileMapHeight) };
                int endX { std::clamp(static_cast<int>(ceilf(maxWorld.x * invTileSize)), 0, g_TileMapWidth) };
                int endY { std::clamp(static_cast<int>(ceilf(maxWorld.y * invTileSize)), 0, g_TileMapHeight) };

                for (int y = startY; y < endY; y++)
                {
                    for (int x = startX; x < endX; x++)
                    {
                        Tile tile { g_TileMap[x][y] };
                        if (tile.type == TileType::None)
                        {
                            continue;
                        }
                        RectF32 dest { static_cast<float>(x * g_TileSize), static_cast<float>(y * g_TileSize), g_TileSize, g_TileSize };
                        Renderer::DrawSprite(g_TileTextures[static_cast<size_t>(tile.type)], dest);
                    }
                }

                for (const auto& entity : gameState.entities)
                {
                    size_t idx = entity.animationIdx;
                    uint32_t height { entity.animations[idx].frameHeight };
                    uint32_t width { entity.animations[idx].frameWidth };

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

                    Renderer::DrawSprite(entity.texture, destination, source);
                }

                for (const auto& entity : gameState.entities)
                {
                    if (entity.type != EntityType::Item) continue;

                    RectF32 dest;
                    dest.width = gameState.daisyItemTexture.width;
                    dest.height = gameState.daisyItemTexture.height;
                    dest.x = entity.position.x;
                    dest.y = entity.position.y;
                    Renderer::DrawSprite(entity.texture, dest);
                }

                if (gameState.activeEntity)
                {
                    RectF32 dest;
                    dest.width = gameState.interactableTexture.width;
                    dest.height = gameState.interactableTexture.height;
                    dest.x = gameState.activeEntity->position.x;
                    dest.y = gameState.activeEntity->position.y;
                    Renderer::DrawSprite(gameState.interactableTexture, dest);
                }

            Renderer::EndMode();

            Renderer::BeginModeScreenSpace();

                Renderer::DrawText(gameState.font1, "FPS: " + std::to_string(Renderer::GetFPS()), 5, 5, gameState.font1.size);
                Renderer::DrawText(gameState.font1, "Sprite Batch Count: " + std::to_string(D3D11::drawCallCount), 5, 15, gameState.font1.size);

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
                        RectF32 entityRect { golem->position.x, golem->position.y, static_cast<float>(golem->animations[idx].frameWidth), static_cast<float>(golem->animations[idx].frameHeight) };
                        if (CheckCollisionPointInRect(virtualWorldPosition, entityRect))
                        {
                            Renderer::DrawText(gameState.font1, "Hovered", virtualMousePosition.x, virtualMousePosition.y, gameState.font1.size);
                        }
                    }
                }

            Renderer::EndMode();

        Renderer::EndFrame();
    }

    void UpdateAndDrawFrame(float deltaTime)
    {
        gameState.tick++;

        Update(deltaTime);
        DrawFrame(deltaTime);
    }
}