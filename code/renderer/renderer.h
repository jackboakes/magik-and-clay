#pragma once

#include "common/types.h"
#include "camera/camera.h"

#include <string_view>
#include <vector>
#include <array>
#include <filesystem>

// opaque handle for texture
struct Texture 
{ 
    U32 handle;
    S32 width;
    S32 height;
};

struct SpriteInstance
{
    Texture texture;
    RectF32 destination;
    RectF32 source;
};

struct RenderPass
{
    HMM_Mat4 viewProjection;
    std::vector<SpriteInstance> sprites;
};

struct Glyph
{
    RectF32 source;
    F32     offsetX;
    F32     offsetY;
    F32     advanceX;
};

// TODO:: This is not shippable but it works for now
constexpr S32 g_fontAtlasWidth = 256;
constexpr S32 g_fontAtlasHeight = 256;
constexpr S32 g_fontFirstChar = ' ';   // space
constexpr S32 g_fontCharCount = '~' - ' ' + 1;   // ASCII 32..126

struct Font
{
    Texture atlas;
    S32     size;
    F32     lineHeight;
    std::array<Glyph, g_fontCharCount> glyphs;
};

namespace Renderer
{
    void WindowCreate(S32 width, S32 height, std::wstring_view title);

    S32 GetFPS();
    U32 GetDrawCallCount();
    RectF32 GetScreenRect();

    Texture LoadTexture(std::string_view path);

    void DrawSprite(Texture texture, const RectF32& destination);
    void DrawSprite(Texture texture, const RectF32& destination, const RectF32& source);

    Font LoadFont(std::filesystem::path filePath, F32 size);
    void DrawText(const Font& font, std::string_view text, F32 x, F32 y, S32 size);

    void BeginFrame(F32 width, F32 height);
    
    void BeginModeScreenSpace();
    void BeginModeWorldSpace(const Camera& camera);
    void EndMode();

    void EndFrame();
}