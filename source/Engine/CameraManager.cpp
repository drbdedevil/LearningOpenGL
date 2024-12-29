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
