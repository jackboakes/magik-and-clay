#pragma once

#include "common/types.h"
#include "camera/camera.h"

#include <string_view>
#include <array>
#include <filesystem>

// opaque handle for texture
struct Texture 
{ 
    U32 handle;
    S32 width;
    S32 height;
};

struct Colour
{
    U8 r;
    U8 g;
    U8 b;
    U8 a { 255 };
};

struct SpriteInstance
{
    Texture texture;
    Vec3F32 position;
    F32 width;
    F32 height;
    RectF32 source;
    Colour colour;
};

inline constexpr size_t g_maxSpritesPerPass { 2048 };

struct RenderPass
{
    HMM_Mat4 viewProjection;
    std::array<SpriteInstance, g_maxSpritesPerPass> sprites;
    size_t spriteCount { 0 };
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

    void DrawSprite(Texture texture, Vec3F32 position, F32 width, F32 height, Colour tint);
    void DrawSprite(Texture texture, Vec3F32 position, float width, float height, const RectF32& source, Colour tint);

    Font LoadFont(std::filesystem::path filePath, F32 size);
    void DrawText(const Font& font, std::string_view text, F32 x, F32 y, S32 size);

    void BeginFrame(F32 width, F32 height);
    
    void BeginModeScreenSpace();
    void BeginModeWorldSpace(const Camera& camera);
    void EndMode();

    void EndFrame();
}