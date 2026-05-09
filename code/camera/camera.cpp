#include "camera.h"

static constexpr float g_NearPlane { 0.0f };
static constexpr float g_FarPlane { 100.0f };

static HMM_Mat4 ViewFromCamera(const Camera& camera)
{
    HMM_Mat4 offset { HMM_Translate({ camera.offset.x, camera.offset.y, 0.0f }) };
    HMM_Mat4 scale { HMM_Scale({ camera.zoom, camera.zoom, 1.0f }) };
    HMM_Mat4 translate { HMM_Translate({ -camera.position.x, -camera.position.y, 0.0f }) };

    return HMM_MulM4(offset, HMM_MulM4(scale, translate));
}

static HMM_Mat4 InverseViewFromCamera(const Camera& camera)
{
    HMM_Mat4 offset { HMM_Translate({ -camera.offset.x, -camera.offset.y, 0.0f }) };
    HMM_Mat4 scale { HMM_Scale({ 1.0f / camera.zoom, 1.0f / camera.zoom, 1.0f }) };
    HMM_Mat4 translate { HMM_Translate({ camera.position.x, camera.position.y, 0.0f }) };

    return HMM_MulM4(translate, HMM_MulM4(scale, offset));
}

HMM_Mat4 ViewProjectionFromCamera(const Camera& camera, float width, float height)
{
    HMM_Mat4 projection { HMM_Orthographic_RH_ZO(
        0.0f, width,
        height, 0.0f,
        g_NearPlane, g_FarPlane
    ) };

    return HMM_MulM4(projection, ViewFromCamera(camera));
}

Vec2F32 ScreenToWorld(Vec2F32 position, const Camera& camera)
{
    HMM_Vec4 result { HMM_MulM4V4(InverseViewFromCamera(camera), { position.x, position.y, 0, 1 }) };
    return { result.X, result.Y };
}

Vec2F32 WorldToScreen(Vec2F32 position, const Camera& camera)
{
    HMM_Vec4 result { HMM_MulM4V4(ViewFromCamera(camera), { position.x, position.y, 0, 1 }) };
    return { result.X, result.Y };
}