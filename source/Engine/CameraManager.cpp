#include "CameraManager.h"

CameraManager* CameraManager::Camera = nullptr;

CameraManager* CameraManager::GetCameraManager()
{
    if (!Camera)
    {
        Camera = new CameraManager();
    }

    return Camera;
}

void CameraManager::AddCameraPosition(const glm::vec3& AddPosition)
{
    Position = Position + AddPosition;
    Point = Point + AddPosition;
}

void CameraManager::DeleteCamera()
{
    if (Camera)
    {
        delete Camera;
    }
}
