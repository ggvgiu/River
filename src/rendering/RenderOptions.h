#pragma once
#include "game/RiverCourse.h"

enum class CameraAngle { Low, Medium, High };

// Previously generated scenery remains visible behind the camera's playfield.
inline constexpr int ValleyHistoryLines = RiverRaid::RetainedSceneLines;

// Presentation settings live outside the game model and never affect collisions.
struct RenderOptions
{
    bool valley3D = true;
    bool seedColors = true;
    CameraAngle cameraAngle = CameraAngle::High;
};
