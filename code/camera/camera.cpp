#include "camera.h"

HMM_Mat4 ViewProjectionFromCamera(const Camera& camera, float width, float height)
{
    HMM_Mat4 projection { HMM_Orthographic_RH_ZO(
        0.0f, width / camera.zoom,
        height / camera.zoom, 0.0f,
        0.0f, 100.0f
    ) };

    // - NOTE: The camera origin is the centre
    HMM_Mat4 view { HMM_Translate(HMM_V3(
        -camera.position.X + (width / camera.zoom) * 0.5f,
        -camera.position.Y + (height / camera.zoom) * 0.5f,
        0.0f
    )) };

    return HMM_MulM4(projection, view);
}