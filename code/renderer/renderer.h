#pragma once

#include "common/types.h"
#include "camera/camera.h"

#include <string_view>
#include <cstdint>

// opaque handle for texture
struct Texture { uint32_t handle; };

struct SpriteInstance
{
    Texture texture;
    RectF32 destination;
};

namespace Renderer
{
    void WindowCreate(int width, int height, std::wstring_view title);

    Texture LoadTexture(std::string_view path);

    void DrawSprite(Texture texture, RectF32 destination);

    void BeginFrame(const Camera& camera);

    void EndFrame();
}