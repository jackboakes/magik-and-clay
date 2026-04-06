#pragma once

#include "HandmadeMath.h"
#include <cstdint>

#include <string_view>
#include "common/types.h"


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

    void BeginFrame();

    void EndFrame();
}