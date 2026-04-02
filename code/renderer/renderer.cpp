#include "renderer.h"
#include "renderer/backend/d3d11_backend.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <vector>

namespace Renderer
{
    static constexpr float VIRTUAL_HEIGHT = 360.0f;

    static HWND windowHandle;
    static std::vector<SpriteInstance> spriteQueue;
    static HMM_Mat4 viewProjection;


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

    void DrawSprite(Texture texture, RectF32 destination)
    {
        SpriteInstance sprite;
        sprite.texture = texture;
        sprite.destination = destination;
        spriteQueue.push_back(sprite);
    }

    void BeginFrame()
    {
        spriteQueue.clear();
        D3D11::BeginFrame();

        RECT clientRect {};
        GetClientRect(D3D11::window.handle, &clientRect);
        UINT width = clientRect.right - clientRect.left;
        UINT height = clientRect.bottom - clientRect.top;

        float aspect = (float)width / (float)height;
        float virtualWidth = VIRTUAL_HEIGHT * aspect;

        HMM_Mat4 view { HMM_M4D(1.0f) };
        HMM_Mat4 projection { HMM_Orthographic_RH_ZO(
            0.0f, virtualWidth,
            VIRTUAL_HEIGHT, 0.0f,
            0.0f, 100.0f
        ) };

        viewProjection = HMM_MulM4(projection, view);
    }

    void EndFrame()
    {
        D3D11::EndFrame(spriteQueue, viewProjection);
    }

}