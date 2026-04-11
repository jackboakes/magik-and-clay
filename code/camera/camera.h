#pragma once
#include "HandmadeMath.h"

struct Camera
{
    HMM_Vec2 position;
    float zoom;
};

HMM_Mat4 ViewProjectionFromCamera(const Camera& camera, float width, float height);