#include "game.h"

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>


#include "win32/win_core.h" // TODO:: remove this depenedency on win32 layer

#include "input/input.h"

#include "renderer/backend/d3d11_backend.h" // For temporary debug code to get sprite batch count


namespace Game
{
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
                case 4: tile.type = TileType::Cauldron;    break;
                case 0: tile.type = TileType::Crop;        break;
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

    void Init()
    {
        Renderer::WindowCreate(1280, 720, L"Farming Sim Prototype");



        Entity golem {};
        golem.type = EntityType::Golem;
        golem.texture = Renderer::LoadTexture("../data/textures/golem.png");
        golem.position = { 1280.0f, 750.0f }; // Near the cauldron in the center of the map


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

        gameState.entities[0] = golem;
        
        g_TileTextures[static_cast<size_t>(TileType::Cauldron)] = Renderer::LoadTexture("../data/textures/cauldron.png");
        g_TileTextures[static_cast<size_t>(TileType::Crop)] = Renderer::LoadTexture("../data/textures/crop.png");
        g_TileTextures[static_cast<size_t>(TileType::Grass_1)] = Renderer::LoadTexture("../data/textures/grass1.png");
        g_TileTextures[static_cast<size_t>(TileType::Grass_2)] = Renderer::LoadTexture("../data/textures/grass2.png");
        g_TileTextures[static_cast<size_t>(TileType::Grass_3)] = Renderer::LoadTexture("../data/textures/grass3.png");

        LoadTileMap("../data/tilemap/tilemap1.csv");

        gameState.font1 = Renderer::LoadFont("../data/font/romulus.ttf", 16.0f);
        gameState.font2 = Renderer::LoadFont("../data/font/tiny5.ttf", 8.0f);

        gameState.camera.position = { (g_TileMapWidth * g_TileSize) * 0.5f, (g_TileMapHeight * g_TileSize) * 0.5f };
        gameState.camera.offset = { 640 * 0.5f, 360 * 0.5f };
        gameState.camera.zoom = 1.0f;
    }

    void Update(float deltaTime)
    {
        Input::ProcessEvents();


        if (Input::IsKeyDown(Key::W))
        {
            gameState.camera.position.Y -= 5.0f;
        }
        if (Input::IsKeyDown(Key::A))
        {
            gameState.camera.position.X -= 5.0f;
        }
        if (Input::IsKeyDown(Key::S))
        {
            gameState.camera.position.Y += 5.0f;
        }
        if (Input::IsKeyDown(Key::D))
        {
            gameState.camera.position.X += 5.0f;
        }


        // TODO:: abstract over the clientrect from window
        RectF32 clientRect { W32::ClientRectFromWindow(D3D11::window.handle) };
        float aspectRatio { clientRect.width / clientRect.height };
        gameState.virtualScreenWidth =  gameState.virtualScreenHeight * aspectRatio;
        gameState.camera.offset = { gameState.virtualScreenWidth * 0.5f, gameState.virtualScreenHeight * 0.5f };

        float scrollWheelDelta { Input::GetScrollDelta() };
        if (scrollWheelDelta != 0.0f)
        {
            HMM_Vec2 mousePos { Input::GetMousePosition() };

            // NOTE:: virtual positions are on the game side of the code
            HMM_Vec2 virtualMousePos;
            virtualMousePos.X = mousePos.X * (gameState.virtualScreenWidth / clientRect.width);
            virtualMousePos.Y = mousePos.Y * (gameState.virtualScreenHeight / clientRect.height);

            HMM_Vec2 previousWorldPos { ScreenToWorld(virtualMousePos, gameState.camera) };

            // TODO:: Figure out what amount of camera steps I want and how far each step will be,
            // linear steps should be the best for this type of game.
            gameState.camera.zoom += scrollWheelDelta / 10.0f;
            gameState.camera.zoom = std::clamp(gameState.camera.zoom, 0.5f, 1.5f);

            HMM_Vec2 postZoomWorldPos { ScreenToWorld(virtualMousePos, gameState.camera) };

            gameState.camera.position.X += (previousWorldPos.X - postZoomWorldPos.X);
            gameState.camera.position.Y += (previousWorldPos.Y - postZoomWorldPos.Y);
        }

        for (auto& entity : gameState.entities)
        {
            if (entity.type != EntityType::Golem) continue;
            uint64_t frame { (gameState.tick / entity.animations[0].frameAdvancement) % entity.animations[0].frameCount };
            entity.animations[0].currentFrame = frame;
        }
    }

    void DrawFrame(float deltaTime)
    {
        Renderer::BeginFrame(gameState.virtualScreenWidth, gameState.virtualScreenHeight);

            Renderer::BeginModeWorldSpace(gameState.camera);

                HMM_Vec2 minWorld { ScreenToWorld({ 0, 0 }, gameState.camera) };
                HMM_Vec2 maxWorld { ScreenToWorld({ gameState.virtualScreenWidth, gameState.virtualScreenHeight }, gameState.camera) };

                static constexpr float invTileSize { 1.0f / g_TileSize };
                int startX { std::clamp(static_cast<int>(floorf(minWorld.X * invTileSize)), 0, g_TileMapWidth) };
                int startY { std::clamp(static_cast<int>(floorf(minWorld.Y * invTileSize)), 0, g_TileMapHeight) };
                int endX { std::clamp(static_cast<int>(ceilf(maxWorld.X * invTileSize)), 0, g_TileMapWidth) };
                int endY { std::clamp(static_cast<int>(ceilf(maxWorld.Y * invTileSize)), 0, g_TileMapHeight) };

                for (int y = startY; y < endY; y++)
                {
                    for (int x = startX; x < endX; x++)
                    {
                        Tile tile { g_TileMap[x][y] };
                        if (tile.type == TileType::None)
                        {
                            continue;
                        }

                        if (tile.type == TileType::Cauldron)
                        {
                            // Only draw on the top-left cell of the 2x2 block
                            if ((x % 2 == 0) && (y % 2 == 0))
                            {
                                RectF32 dest { static_cast<float>(x * g_TileSize), static_cast<float>(y * g_TileSize), g_TileSize * 2, g_TileSize * 2 };
                                Renderer::DrawSprite(g_TileTextures[static_cast<size_t>(TileType::Cauldron)], dest);
                            }
                            continue;
                        }

                        RectF32 dest { static_cast<float>(x * g_TileSize), static_cast<float>(y * g_TileSize), g_TileSize, g_TileSize };
                        Renderer::DrawSprite(g_TileTextures[static_cast<size_t>(tile.type)], dest);
                    }
                }

                for (const auto& entity : gameState.entities)
                {
                    if (entity.type != EntityType::Golem) continue;
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
                    destination.x = entity.position.X;
                    destination.y = entity.position.Y;

                    Renderer::DrawSprite(entity.texture, destination, source);
                }

            Renderer::EndMode();

            Renderer::BeginModeScreenSpace();

                Renderer::DrawText(gameState.font1, "FPS: " + std::to_string(Renderer::GetFPS()), 5, 5, gameState.font1.size);
                Renderer::DrawText(gameState.font2, "Sprite Batch Count: " + std::to_string(D3D11::drawCallCount), 5, 15, gameState.font2.size);

            Renderer::EndMode();

        Renderer::EndFrame();
    }

    void UpdateAndDrawFrame(float deltaTime)
    {
        gameState.tick++;
        // TODO:: cap to 60 FPS/UPS
        Update(deltaTime);
        DrawFrame(deltaTime);
    }
}