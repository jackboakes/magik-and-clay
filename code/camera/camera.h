#pragma once

#include "common/types.h"
#include "HandmadeMath.h"

struct Camera
{
    Vec2F32 position;
    Vec2F32 offset;
    F32 zoom;
};

HMM_Mat4 ViewProjectionFromCamera(const Camera& camera, F32 width, F32 height);
Vec2F32 WorldFromScreen(Vec2F32 position, const Camera& camera);
Vec2F32 ScreenFromWorld(Vec2F32 position, const Camera& camera);