#pragma once

enum class CameraAngle { Low, Medium, High };

// Previously generated scenery remains visible behind the camera's playfield.
inline constexpr int ValleyHistoryLines = 128;

// Presentation settings live outside the game model and never affect collisions.
struct RenderOptions
{
    bool valley3D = false;
    CameraAngle cameraAngle = CameraAngle::Medium;
};
