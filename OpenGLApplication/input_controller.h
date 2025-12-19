// Keyboard-handling
// Tracks current bone-index

#pragma once
#include "camera.h"

class InputController
{
public:
    InputController();

    void Update(float deltaTime);

    int GetCurrentBoneIndex() const;

    Camera& GetCamera();

private:
    Camera camera;
    int currentBoneIndex;
};
