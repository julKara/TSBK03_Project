// Keyboard-handling
// Tracks current bone-index

#pragma once
#include "camera.h"

class InputController
{
public:
    InputController();

    void Update(float deltaTime);

    // Sets the maximum valid bone index (exclusive)
    void SetMaxBoneIndex(int maxIndex);
    
    int GetCurrentBoneIndex() const;

    Camera& GetCamera();

    // Tracks previous key states to detect single key presses
    bool prevUpPressed = false;
    bool prevDownPressed = false;
    
    // Mouse drag rotation ----------------

    // True while left mouse button is held
    bool isDragging = false;

    // Last known mouse position (screen space)
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    // Accumulated rotation angles (radians)
    float modelYaw = 0.0f;   // Rotation around Y-axis
    float modelPitch = 0.0f; // Rotation around X-axis

    // Sensitivity factor for drag rotation
    float mouseSensitivity = 0.005f;

    // Returns a rotation matrix for the model
    glm::mat4 GetModelRotationMatrix() const;

private:
    Camera camera;
    int currentBoneIndex;
    // Maximum valid bone index(set externally)
    int maxBoneIndex = 0;
};
