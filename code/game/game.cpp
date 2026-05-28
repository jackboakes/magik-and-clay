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

        gameState.g_GolemTexture = Renderer::LoadTexture("../data/textures/golem.png");

        Entity golem {};
        golem.type = EntityType::Golem;
        golem.texture = gameState.g_GolemTexture;
        golem.position = { 1280.0f, 750.0f }; // Near the cauldron in the center of the map
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
        carrying.frameAdvancement = 2;

        golem.animations[static_cast<int>(GolemState::Idle)] = idle;
        golem.animations[0] = idle; // carrying animation is tied to bool and not state
        golem.animations[1] = carrying;

        gameState.entities.push_back(golem);
        
        Texture cauldronTexture { Renderer::LoadTexture("../data/textures/cauldron.png") };
        g_TileTextures[static_cast<size_t>(TileType::Grass_1)] = Renderer::LoadTexture("../data/textures/grass1.png");
        g_TileTextures[static_cast<size_t>(TileType::Grass_2)] = Renderer::LoadTexture("../data/textures/grass2.png");
        g_TileTextures[static_cast<size_t>(TileType::Grass_3)] = Renderer::LoadTexture("../data/textures/grass3.png");

        Entity cauldron {};
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

        cauldron.animations[0] = cauldronAnimation;
        gameState.entities.push_back(cauldron);

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
            gameState.camera.position.y -= 5.0f;
        }
        if (Input::IsKeyDown(Key::A))
        {
            gameState.camera.position.x -= 5.0f;
        }
        if (Input::IsKeyDown(Key::S))
        {
            gameState.camera.position.y += 5.0f;
        }
        if (Input::IsKeyDown(Key::D))
        {
            gameState.camera.position.x += 5.0f;
        }

        if (Input::IsKeyPressed(Key::G))
        {
            Vec2F32 mousePosition { Input::GetMousePosition() };
            Vec2F32 virtualMousePosition { VirtualPositonFromScreenPoint(mousePosition) };
            Vec2F32 virtualWorldPosition { WorldFromScreen(virtualMousePosition, gameState.camera) };
            Vec2S32 gridPosition { TileCoordinateFromPoint(virtualWorldPosition) };
            
            if (gridPosition.x >= 0 && gridPosition.x < g_TileMapWidth && gridPosition.y >= 0 && gridPosition.y < g_TileMapHeight && !CheckCollidableFromTile(gridPosition))
            {
                Entity golem {};
                golem.type = EntityType::Golem;
                golem.texture = gameState.g_GolemTexture;
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
                carrying.frameAdvancement = 2;

                golem.animations[static_cast<int>(GolemState::Idle)] = idle;
                golem.animations[0] = idle; // carrying animation is tied to bool and not state
                golem.animations[1] = carrying;

                gameState.entities.push_back(golem);
            }
        }


        RectF32 screenRect { Renderer::GetScreenRect() };
        float aspectRatio { screenRect.width / screenRect.height };
        gameState.virtualScreenWidth =  gameState.virtualScreenHeight * aspectRatio;
        gameState.camera.offset = { gameState.virtualScreenWidth * 0.5f, gameState.virtualScreenHeight * 0.5f };

        float scrollWheelDelta { Input::GetScrollDelta() };
        if (scrollWheelDelta != 0.0f)
        {
            Vec2F32 mousePos { Input::GetMousePosition() };
            Vec2F32 virtualMousePos { VirtualPositonFromScreenPoint(mousePos) };

            Vec2F32 previousWorldPos { WorldFromScreen(virtualMousePos, gameState.camera) };

            // TODO:: Figure out what amount of camera steps I want and how far each step will be,
            // linear steps should be the best for this type of game.
            gameState.camera.zoom += scrollWheelDelta / 10.0f;
            gameState.camera.zoom = std::clamp(gameState.camera.zoom, 0.5f, 1.5f);

            Vec2F32 postZoomWorldPos { WorldFromScreen(virtualMousePos, gameState.camera) };

            gameState.camera.position.x += (previousWorldPos.x - postZoomWorldPos.x);
            gameState.camera.position.y += (previousWorldPos.y - postZoomWorldPos.y);
        }

        for (auto& entity : gameState.entities)
        {
            if (entity.type != EntityType::Golem) continue;
            entity.animationTicks++;
            uint64_t frame { (entity.animationTicks / entity.animations[0].frameAdvancement) % entity.animations[0].frameCount };
            entity.animations[0].currentFrame = static_cast<uint32_t>(frame);
        }

        for (auto& entity : gameState.entities)
        {
            if (entity.type != EntityType::Cauldron) continue;
            entity.animationTicks++;
            uint64_t frame { (entity.animationTicks / entity.animations[0].frameAdvancement) % entity.animations[0].frameCount };
            entity.animations[0].currentFrame = static_cast<uint32_t>(frame);
        }

        for (auto& entity : gameState.entities)
        {
            if (entity.type != EntityType::Crop) continue;
            if (entity.growthTicks >= 1200) continue;

            entity.growthTicks++;
            int stage { static_cast<int>(entity.growthTicks / 300) };
            entity.animations[0].currentFrame = std::clamp(stage, 0, 3);
            if (stage >= 3)
            {
                entity.harestable = true;
            }
        }

        for (auto& crop : gameState.entities)
        {
            if (crop.type != EntityType::Crop) continue;
            if (!crop.harestable) continue;
            if (crop.cropTaken) continue;

            for (auto& golem : gameState.entities)
            {
                if (golem.type != EntityType::Golem) continue;
                if (golem.golemState != GolemState::Idle) continue;
                if (!golem.path.empty()) continue; 

                crop.cropTaken = true;
                golem.path = FindPath(TileCoordinateFromPoint(golem.position), TileCoordinateFromPoint(crop.position));
                golem.golemState = GolemState::Pathing;
                break;
            }
        }

        for (auto& entity : gameState.entities)
        {
            if (entity.type != EntityType::Golem) continue;
            if (entity.path.empty()) continue;
            
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
            }
            else
            {
                entity.position.x += (delta.x / distance) * step;
                entity.position.y += (delta.y / distance) * step;
            }
        }
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
                    uint32_t height { entity.animations[0].frameHeight };
                    uint32_t width { entity.animations[0].frameWidth };

                    RectF32 source { 0 };
                    source.width = width;
                    source.height = height;
                    source.x = entity.animations[0].xOffset + (entity.animations[0].currentFrame * (entity.animations[0].xOffset + width));
                    source.y = entity.animations[0].yOffset;

                    RectF32 destination { 0 };
                    destination.width = width;
                    destination.height = height;
                    destination.x = entity.position.x;
                    destination.y = entity.position.y;

                    Renderer::DrawSprite(entity.texture, destination, source);
                }

            Renderer::EndMode();

            Renderer::BeginModeScreenSpace();

                Renderer::DrawText(gameState.font1, "FPS: " + std::to_string(Renderer::GetFPS()), 5, 5, gameState.font1.size);
                Renderer::DrawText(gameState.font1, "Sprite Batch Count: " + std::to_string(D3D11::drawCallCount), 5, 15, gameState.font1.size);

                {
                    Vec2F32 mousePosition { Input::GetMousePosition() };
                    Vec2F32 virtualMousePosition { VirtualPositonFromScreenPoint(mousePosition) };
                    Vec2F32 virtualWorldPosition { WorldFromScreen(virtualMousePosition, gameState.camera) };
                    Vec2S32 gridPosition { TileCoordinateFromPoint(virtualWorldPosition) };
                    Renderer::DrawText(gameState.font1, "Mouse Grid Position: " + std::to_string(gridPosition.x) + ", " + std::to_string(gridPosition.y), 5, 25, gameState.font1.size);
                }

                for (const auto& entity : gameState.entities)
                {
                    if (entity.type != EntityType::Golem) continue;
                    Vec2F32 mousePosition { Input::GetMousePosition() };
                    Vec2F32 virtualMousePosition { VirtualPositonFromScreenPoint(mousePosition) };
                    Vec2F32 virtualWorldPosition { WorldFromScreen(virtualMousePosition, gameState.camera) };
                    RectF32 entityRect { entity.position.x, entity.position.y, static_cast<float>(entity.animations[0].frameWidth), static_cast<float>(entity.animations[0].frameHeight) };
                    if (CheckCollisionPointInRect(virtualWorldPosition, entityRect))
                    {
                        Renderer::DrawText(gameState.font1, "Hovered", virtualMousePosition.x, virtualMousePosition.y, gameState.font1.size);
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