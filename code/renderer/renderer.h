#pragma once

#include "common/types.h"
#include "camera/camera.h"

#include <string_view>
#include <cstdint>
#include <vector>
#include <array>
#include <filesystem>

// opaque handle for texture
struct Texture 
{ 
    uint32_t handle;
    int width;
    int height;
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
    float   offsetX;
    float   offsetY;
    float   advanceX;
};

// TODO:: This is not shippable but it works for now
constexpr int g_fontAtlasWidth = 256;
constexpr int g_fontAtlasHeight = 256;
constexpr int g_fontFirstChar = ' ';   // space
constexpr int g_fontCharCount = '~' - ' ' + 1;   // ASCII 32..126

struct Font
{
    Texture atlas;
    int     size;
    float   lineHeight;
    std::array<Glyph, g_fontCharCount> glyphs;
};


#undef DrawText
namespace Renderer
{
    void WindowCreate(int width, int height, std::wstring_view title);

    Texture LoadTexture(std::string_view path);

    void DrawSprite(Texture texture, const RectF32& destination);
    void DrawSprite(Texture texture, const RectF32& destination, const RectF32& source);

    Font LoadFont(std::filesystem::path filePath, float size);
    void DrawText(const Font& font, std::string_view text, float x, float y, int size);

    void BeginFrame(float width, float height);
    
    void BeginModeScreenSpace();
    void BeginModeWorldSpace(const Camera& camera);
    void EndMode();

    void EndFrame();
}