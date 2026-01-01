#pragma once
#include "camera.h"

/*
* Handles all interaction with the program. Possible
* interactions include:
*   * W A S D - keys for x- y-axis camera movement
*   * Q E - key for z-axis camera movement
*   * Up Down Arrows - increase/decrease current bone-index (weight-viz)
*   * Dragging - Rotate model
*/

class InputController
{
public:
    InputController();

    void Update(float deltaTime);

    // Sets the maximum valid bone index
    void SetMaxBoneIndex(int maxIndex);
    
    
    // Get-functions ----------------
    int GetCurrentBoneIndex() const;

    Camera& GetCamera();
    
    glm::mat4 GetModelRotationMatrix() const;

    // Button presses ----------------
    
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

private:

    Camera camera;
    int currentBoneIndex;
    
    // Maximum valid bone index(set in Main)
    int maxBoneIndex = 0;
};
