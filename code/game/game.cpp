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

        Texture golem { Renderer::LoadTexture("../data/textures/idle_golem.png") };
        
        g_TileTextures[static_cast<size_t>(TileType::Cauldron)] = Renderer::LoadTexture("../data/textures/cauldron.png");
        g_TileTextures[static_cast<size_t>(TileType::Crop)] = Renderer::LoadTexture("../data/textures/crop.png");
        g_TileTextures[static_cast<size_t>(TileType::Grass_1)] = Renderer::LoadTexture("../data/textures/grass1.png");
        g_TileTextures[static_cast<size_t>(TileType::Grass_2)] = Renderer::LoadTexture("../data/textures/grass2.png");
        g_TileTextures[static_cast<size_t>(TileType::Grass_3)] = Renderer::LoadTexture("../data/textures/grass3.png");

        LoadTileMap("../data/tilemap/tilemap1.csv");

        gameState.font1 = Renderer::LoadFont("../data/font/romulus.ttf", 16.0f);
        gameState.font2 = Renderer::LoadFont("../data/font/tiny5.ttf", 8.0f);

        RectF32 golemDest;
        golemDest.height = 16;
        golemDest.width = 16;
        golemDest.x = 64;
        golemDest.y = 64;

        gameState.camera.position = { 0.0f, 0.0f };
        gameState.camera.offset = { 640 * 0.5f, 360 * 0.5f };
        gameState.camera.zoom = 1.0f;
    }

    void Update()
    {
        Input::ProcessEvents();


        if (Input::IsKeyDown(Key::W))
        {
            gameState.camera.position.Y -= 1.0f;
        }
        if (Input::IsKeyDown(Key::A))
        {
            gameState.camera.position.X -= 1.0f;
        }
        if (Input::IsKeyDown(Key::S))
        {
            gameState.camera.position.Y += 1.0f;
        }
        if (Input::IsKeyDown(Key::D))
        {
            gameState.camera.position.X += 1.0f;
        }


        // TODO:: abstract over the clientrect from window
        RectF32 clientRect { W32::ClientRectFromWindow(D3D11::window.handle) };
        float aspectRatio { clientRect.width / clientRect.height };
        gameState.virtualScreenWidth =  gameState.virtualScreenHeight * aspectRatio;
        gameState.camera.offset = { gameState.virtualScreenWidth * 0.5f, gameState.virtualScreenWidth * 0.5f };

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
    }

    void DrawFrame()
    {
        Renderer::BeginFrame(gameState.virtualScreenWidth, gameState.virtualScreenHeight);

            Renderer::BeginModeWorldSpace(gameState.camera);

                static constexpr float g_MapOffsetX { -(g_TileMapWidth * g_TileSize) * 0.5f };
                static constexpr float g_MapOffsetY { -(g_TileMapHeight * g_TileSize) * 0.5f };

                HMM_Vec2 minWorld = ScreenToWorld({ 0, 0 }, gameState.camera);
                HMM_Vec2 maxWorld = ScreenToWorld({ gameState.virtualScreenWidth, gameState.virtualScreenHeight }, gameState.camera);

                int startX = std::clamp(static_cast<int>(floorf((minWorld.X - g_MapOffsetX) / g_TileSize)), 0, g_TileMapWidth);
                int startY = std::clamp(static_cast<int>(floorf((minWorld.Y - g_MapOffsetY) / g_TileSize)), 0, g_TileMapHeight);
                int endX = std::clamp(static_cast<int>(ceilf((maxWorld.X - g_MapOffsetX) / g_TileSize)), 0, g_TileMapWidth);
                int endY = std::clamp(static_cast<int>(ceilf((maxWorld.Y - g_MapOffsetY) / g_TileSize)), 0, g_TileMapHeight);

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
                            // Tiled exports row-major, so the top-left is the smallest x,y
                            // Check we're at the origin cell (even grid position, or track it explicitly)
                            // Simple approach: draw if this is the top-left of a 2x2 block
                            if ((x % 2 == 0) && (y % 2 == 0)) // adjust parity to match your map
                            {
                                RectF32 dest { g_MapOffsetX + x * g_TileSize, g_MapOffsetY + y * g_TileSize, g_TileSize * 2, g_TileSize * 2 };
                                Renderer::DrawSprite(g_TileTextures[static_cast<size_t>(TileType::Cauldron)], dest);
                            }
                            continue;
                        }

                        RectF32 dest { g_MapOffsetX + x * g_TileSize, g_MapOffsetY + y * g_TileSize, g_TileSize, g_TileSize };
                        Renderer::DrawSprite(g_TileTextures[static_cast<size_t>(tile.type)], dest);
                    }
                }

            Renderer::EndMode();

            Renderer::BeginModeScreenSpace();

                Renderer::DrawText(gameState.font1, "FPS: " + std::to_string(Renderer::GetFPS()), 5, 5, gameState.font1.size);
                Renderer::DrawText(gameState.font2, "Sprite Batch Count: " + std::to_string(D3D11::drawCallCount), 5, 15, gameState.font2.size);

            Renderer::EndMode();

        Renderer::EndFrame();
    }

    void UpdateAndDrawFrame()
    {
        // TODO:: cap to 60 FPS/UPS
        Update();
        DrawFrame();
    }
}