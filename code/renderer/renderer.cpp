#include "renderer.h"

#include "renderer/backend/d3d11_backend.h"
#include "HandmadeMath.h"
#include "win32/win_core.h"
#include "common/utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <algorithm>
#include <cassert>
#include <vector>


namespace Renderer
{
    static F32 s_width;
    static F32 s_height;
    static std::vector<RenderPass> s_passes;
    static RenderPass* s_activePass { nullptr };
    static U64 endCurrentFrameTime { 0 };
    static U64 endPreviousFrameTime { 0 };
    static S32 fps { 0 };

    static Texture whitePixel;

    void WindowCreate(S32 width, S32 height, std::wstring_view title)
    {
        s_width = static_cast<F32>(width);
        s_height = static_cast<F32>(height);

        D3D11::window.handle = W32::WindowCreate(width, height, title);
        if (!D3D11::window.handle)
        {
            MessageBoxW(nullptr, L"WIN32: Failed to create window", L"Error", MB_ICONERROR | MB_OK);
            ExitProcess(1);
        }

        D3D11::Init();
        D3D11::WindowEquip(D3D11::window.handle);

        ShowWindow(D3D11::window.handle, SW_SHOW);

        // init white square
        U8 whitePixelData[] { 0xFF, 0xFF, 0xFF, 0xFF };
        U8* whitePix { whitePixelData };
        whitePixel.handle = D3D11::CreateTexture(whitePix, 1, 1);
        whitePixel.width = 1;
        whitePixel.height = 1;
    }

    S32 GetFPS()
    {
        return fps;
    }

    U32 GetDrawCallCount()
    {
        return D3D11::drawCallCount;
    }

    RectF32 GetScreenRect()
    {
        return W32::ClientRectFromWindow(D3D11::window.handle);
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

    void DrawSprite(Texture texture, Vec3F32 position, F32 width, F32 height, Colour tint)
    {
        if (s_activePass)
        {
            SpriteInstance sprite;
            sprite.texture = texture;
            sprite.position = position;
            sprite.width = width;
            sprite.height = height;
            sprite.source.width = width;
            sprite.source.height = height;
            sprite.source.x = 0;
            sprite.source.y = 0;
            sprite.colour = tint;

            assert(g_maxSpritesPerPass != s_activePass->spriteCount && "Attempting to draw into full array of sprites");
            s_activePass->sprites[s_activePass->spriteCount] = sprite;
            s_activePass->spriteCount++;
        }
        // TODO:: logging
    }

    void DrawSprite(Texture texture, Vec3F32 position, float width, float height, const RectF32& source, Colour tint)
    {
        if (s_activePass)
        {
            SpriteInstance sprite;
            sprite.texture = texture;
            sprite.position = position;
            sprite.width = width;
            sprite.height = height;
            sprite.source = source;
            sprite.colour = tint;

            assert(g_maxSpritesPerPass != s_activePass->spriteCount && "Attempting to draw into full array of sprites");
            s_activePass->sprites[s_activePass->spriteCount] = sprite;
            s_activePass->spriteCount++;
        }
        // TODO:: logging
    }

    void DrawRectangle(Vec3F32 position, float width, float height, Colour tint)
    {
        DrawSprite(whitePixel, position, width, height, tint);
    }

    void DrawRectangleLines(Vec3F32 position, float width, float height, S32 thickness, Colour tint)
    {
        //top
        DrawRectangle(position, width, thickness, tint);
        // left
        DrawRectangle({ position.x, position.y + thickness, position.z }, thickness, height - 2 * thickness, tint);
        // bottom
        DrawRectangle({ position.x, position.y + height - thickness, position.z }, width, thickness, tint);
        // right
        DrawRectangle({ position.x + width - thickness, position.y + thickness, position.z }, thickness, height - 2 * thickness, tint);
    }

    Font LoadFont(std::filesystem::path filePath, F32 size)
    {
        auto fontData { LoadFileBinary(filePath) };

        stbtt_fontinfo fontInfo;
        stbtt_InitFont(&fontInfo, reinterpret_cast<const unsigned char*>(fontData->data()), 0);

        F32 scale { stbtt_ScaleForMappingEmToPixels(&fontInfo, size) };

        int ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

        std::array<unsigned char, g_fontAtlasWidth* g_fontAtlasHeight> bitmap;
        std::array<stbtt_packedchar, g_fontCharCount> packedChars {};

        stbtt_pack_context packContext;
        stbtt_PackBegin(&packContext, bitmap.data(), g_fontAtlasWidth, g_fontAtlasHeight, 0, 0, nullptr);

        stbtt_PackSetOversampling(&packContext, 1, 1);
        stbtt_PackFontRange(&packContext, reinterpret_cast<const unsigned char*>(fontData->data()), 0, STBTT_POINT_SIZE(size), g_fontFirstChar, g_fontCharCount, packedChars.data());

        stbtt_PackEnd(&packContext);


        Font font;
        font.size = static_cast<S32>((ascent - descent) * scale);
        font.lineHeight = (ascent - descent + lineGap) * scale;

        for (S32 i { 0 }; i < g_fontCharCount; i++)
        {
            stbtt_aligned_quad quad;

            F32 cx { 0 }, cy { 0 };

            stbtt_GetPackedQuad(packedChars.data(), g_fontAtlasWidth, g_fontAtlasHeight, i, &cx, &cy, &quad, 1);
            font.glyphs[i].source = { quad.s0 * g_fontAtlasWidth, quad.t0 * g_fontAtlasHeight, (quad.s1 - quad.s0) * g_fontAtlasWidth, (quad.t1 - quad.t0) * g_fontAtlasHeight };
            font.glyphs[i].offsetX = quad.x0;
            font.glyphs[i].offsetY = quad.y0 + (ascent * scale);
            font.glyphs[i].advanceX = packedChars[i].xadvance;
        }

        // NOTE:: Need to add space manually since it's glyph doesn't have visible pixels for stbtt_PackFontRange atlas
        S32 spaceIndex { ' ' - g_fontFirstChar };
        int advanceWidth;
        int leftBearing;
        stbtt_GetCodepointHMetrics(&fontInfo, ' ', &advanceWidth, &leftBearing);
        font.glyphs[spaceIndex].advanceX = advanceWidth * scale;

        // Handover atlas to GPU
        std::vector<unsigned char> rgba(g_fontAtlasWidth * g_fontAtlasHeight * 4);
        for (S32 i = 0; i < g_fontAtlasWidth * g_fontAtlasHeight; i++)
        {
            rgba[i * 4 + 0] = 255;
            rgba[i * 4 + 1] = 255;
            rgba[i * 4 + 2] = 255;
            rgba[i * 4 + 3] = bitmap[i];
        }

        font.atlas.handle = D3D11::CreateTexture(rgba.data(), g_fontAtlasWidth, g_fontAtlasHeight);
        font.atlas.width = g_fontAtlasWidth;
        font.atlas.height = g_fontAtlasHeight;

        return font;
    }

    void DrawText(const Font& font, std::string_view text, F32 x, F32 y, S32 size)
    {
        F32 scale { static_cast<F32>(size) / static_cast<F32>(font.size) };

        F32 textOffsetX { x };
        F32 textOffsetY { y };

        for (const char ch : text)
        {
            if (ch < g_fontFirstChar || ch >= g_fontFirstChar + g_fontCharCount)
            {
                continue;
            }

            const Glyph& glpyh { font.glyphs[ch - g_fontFirstChar] };

            RectF32 dest;
            dest.x = floorf(textOffsetX + glpyh.offsetX * scale);
            dest.y = floorf(textOffsetY + glpyh.offsetY * scale);
            dest.width = floorf(glpyh.source.width * scale);
            dest.height = floorf(glpyh.source.height * scale);

            Renderer::DrawSprite(font.atlas, { dest.x, dest.y, 0.0f }, dest.width, dest.height, glpyh.source, {255, 255, 255});

            textOffsetX += glpyh.advanceX * scale;
        }
    }

    void BeginFrame(F32 width, F32 height)
    {
        s_width = width;
        s_height = height;
        s_passes.clear();
        D3D11::BeginFrame();
    }
    
    void BeginModeScreenSpace()
    {
        RenderPass& pass { s_passes.emplace_back() };
        pass.viewProjection = HMM_Orthographic_LH_ZO(0.0f, s_width, s_height, 0.0f, 0.0f, 100.0f);
        s_activePass = &pass;
    }

    void BeginModeWorldSpace(const Camera& camera)
    {
        RenderPass& pass { s_passes.emplace_back() };
        pass.viewProjection = ViewProjectionFromCamera(camera, s_width, s_height);
        s_activePass = &pass;
    }

    void EndMode()
    {
        s_activePass = nullptr;
    }

    void EndFrame()
    {
        for (RenderPass& pass : s_passes)
        {
            // TODO:: could collect the order of textures as the user calls drawsprite instead of logn sorting by handle
            std::sort(pass.sprites.begin(), pass.sprites.begin() + pass.spriteCount, [](const SpriteInstance& a, const SpriteInstance& b) 
            {
                return a.texture.handle > b.texture.handle;
            });
        }

        D3D11::SubmitFrame(s_passes);
  
        D3D11::EndFrame();

        // capture time for fps
        endPreviousFrameTime = endCurrentFrameTime;
        endCurrentFrameTime = W32::TimeMicroseconds();

        U64 frameTimeMicroseconds { endCurrentFrameTime - endPreviousFrameTime };

        static S32 frameCount { 0 };
        static U64 accumulatedTime { 0 };

        accumulatedTime += (endCurrentFrameTime - endPreviousFrameTime);
        frameCount++;

        if (accumulatedTime >= 500000) // update every 0.5s
        {
            fps = static_cast<S32>(frameCount * 1000000.0 / accumulatedTime);
            frameCount = 0;
            accumulatedTime = 0;
        }
    }

}