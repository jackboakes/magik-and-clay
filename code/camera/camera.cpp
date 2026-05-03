#include "camera.h"

static constexpr float g_NearPlane { 0.0f };
static constexpr float g_FarPlane { 100.0f };

HMM_Mat4 ViewFromCamera(const Camera& camera)
{
    HMM_Mat4 offset { HMM_Translate({ camera.offset.X, camera.offset.Y, 0.0f }) };
    HMM_Mat4 scale { HMM_Scale({ camera.zoom, camera.zoom, 1.0f }) };
    HMM_Mat4 translate { HMM_Translate({ -camera.position.X, -camera.position.Y, 0.0f }) };

    return HMM_MulM4(offset, HMM_MulM4(scale, translate));
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