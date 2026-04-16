#include "renderer.h"
#include "renderer/backend/d3d11_backend.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vector>

#include "HandmadeMath.h"
#include "win32/win_core.h"


namespace Renderer
{
    static constexpr float VIRTUAL_HEIGHT { 360.0f };

    static std::vector<SpriteInstance> spriteQueue;
    static HMM_Mat4 viewProjection;


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

        Texture tex = D3D11::CreateTexture(textureData, width, height);
        stbi_image_free(textureData);
        return tex;
    }

    void DrawSprite(Texture texture, const RectF32& destination, const RectF32& source = RectF32 {})
    {
        SpriteInstance sprite;
        sprite.texture = texture;
        sprite.destination = destination;
        sprite.source = source;
        spriteQueue.push_back(sprite);
    }

    void BeginFrame(const Camera& camera)
    {
        spriteQueue.clear();
        D3D11::BeginFrame();

        RectF32 clientRect {W32::ClientRectFromWindow(D3D11::window.handle)};

        float aspectRatio = clientRect.width / clientRect.height;
        float virtualWidth = VIRTUAL_HEIGHT * aspectRatio;

        viewProjection = ViewProjectionFromCamera(camera, virtualWidth, VIRTUAL_HEIGHT);
    }

    void EndFrame()
    {
        D3D11::EndFrame(spriteQueue, viewProjection);
    }

}