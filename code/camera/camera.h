#pragma once
#include "common/types.h"
#include "HandmadeMath.h"

struct Camera
{
    Vec2F32 position;
    Vec2F32 offset;
    float zoom;
};

HMM_Mat4 ViewProjectionFromCamera(const Camera& camera, float width, float height);
Vec2F32 ScreenToWorld(Vec2F32 position, const Camera& camera);
Vec2F32 WorldToScreen(Vec2F32 position, const Camera& camera);