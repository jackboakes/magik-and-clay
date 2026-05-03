#pragma once
#include "HandmadeMath.h"

struct Camera
{
    HMM_Vec2 position;
    HMM_Vec2 offset;
    float zoom;
};

HMM_Mat4 ViewProjectionFromCamera(const Camera& camera, float width, float height);
HMM_Vec2 ScreenToWorld(HMM_Vec2 position, const Camera& camera);
HMM_Vec2 WorldToScreen(HMM_Vec2 position, const Camera& camera);