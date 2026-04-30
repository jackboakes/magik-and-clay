#include "renderer.h"
#include "renderer/backend/d3d11_backend.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vector>
#include <algorithm>

#include "HandmadeMath.h"
#include "win32/win_core.h"


namespace Renderer
{
    static float s_width;
    static float s_height;
    static std::vector<RenderPass> s_passes;
    static RenderPass* s_activePass { nullptr };


    void WindowCreate(int width, int height, std::wstring_view title)
    {
        D3D11::window.handle = W32::WindowCreate(width, height, title);
        if (!D3D11::window.handle)
        {
            MessageBoxW(nullptr, L"WIN32: Failed to create window", L"Error", MB_ICONERROR | MB_OK);
            ExitProcess(1);
        }

        D3D11::Init();
        D3D11::WindowEquip(D3D11::window.handle);

        ShowWindow(D3D11::window.handle, SW_SHOW);
    }


    Texture LoadTexture(std::string_view path)
    {
        int width;
        int height;
        unsigned char* textureData { stbi_load(path.data(), &width, &height, nullptr, 4)};

        if (!textureData)
        {
            // TODO:: handle failure properly
            return Texture{0};
        }

        Texture texture;
        texture.handle = D3D11::CreateTexture(textureData, width, height);
        texture.width = width;
        texture.height = height;

        stbi_image_free(textureData);
        return texture;
    }

    void DrawSprite(Texture texture, const RectF32& destination)
    {
        if (s_activePass)
        {
            SpriteInstance sprite;
            sprite.texture = texture;
            sprite.destination = destination;
            sprite.source.width = destination.width;
            sprite.source.height = destination.height;
            sprite.source.x = 0;
            sprite.source.y = 0;
            s_activePass->sprites.push_back(sprite);
        }
        // TODO:: logging
    }

    void DrawSprite(Texture texture, const RectF32& destination, const RectF32& source)
    {
        if (s_activePass)
        {
            SpriteInstance sprite;
            sprite.texture = texture;
            sprite.destination = destination;
            sprite.source = source;
            s_activePass->sprites.push_back(sprite);
        }
        // TODO:: logging
    }

    void BeginFrame(float width, float height)
    {
        s_width = width;
        s_height = height;
        s_passes.clear();
        D3D11::BeginFrame();
    }
    
    void BeginModeScreenSpace()
    {
        RenderPass pass;
        pass.viewProjection = HMM_Orthographic_RH_ZO(0.0f, s_width, s_height, 0.0f, 0.0f, 100.0f);
        s_passes.push_back(pass);
        s_activePass = &s_passes.back();
    }

    void BeginModeWorldSpace(const Camera& camera)
    {
        RenderPass pass;
        pass.viewProjection = ViewProjectionFromCamera(camera, s_width, s_height);
        s_passes.push_back(pass);
        s_activePass = &s_passes.back();
    }

    void EndMode()
    {
        s_activePass = nullptr;
    }

    void EndFrame()
    {
        for (RenderPass& pass : s_passes)
        {
            // TODO:: z layer sorting since this is temp
            std::sort(pass.sprites.begin(), pass.sprites.end(), [](const SpriteInstance& a, const SpriteInstance& b) 
            {
                return a.texture.handle > b.texture.handle;
            });
        }

        D3D11::SubmitFrame(s_passes);
  
        D3D11::EndFrame();
    }

}