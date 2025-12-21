// Read GLFW key-states
// Updates camera and bone-index

#include "input_controller.h"
#include <GLFW/glfw3.h>
#include <algorithm>

// Globally accessible window
extern GLFWwindow* gWindow;

InputController::InputController()
    : currentBoneIndex(0)
{
}

bool IsKeyPressed(int key)
{
    if (!gWindow)
        return false;

    return glfwGetKey(gWindow, key) == GLFW_PRESS;
}

void InputController::Update(float deltaTime)
{ 
    float speed = 5.0f * deltaTime;

    // Normal camera movement(W/ A / S / D) ----------------
    if (IsKeyPressed(GLFW_KEY_W)) {
        //printf("\n Pressed W!\n");
        camera.MoveForward(speed);
    }
    if (IsKeyPressed(GLFW_KEY_S)) {
        //printf("\n Pressed S!\n");
        camera.MoveForward(-speed);
    }
    if (IsKeyPressed(GLFW_KEY_A)) {
        //printf("\n Pressed A!\n");
        camera.MoveRight(-speed);
    }
    if (IsKeyPressed(GLFW_KEY_D)) {
        //printf("\n Pressed D!\n");
        camera.MoveRight(speed);
    }

    // Vertical camera movement (ArrowUp / ArrowDown) ----------------
    // ---------------- Bone index selection ----------------

    // Check current key states
    bool upPressed = IsKeyPressed(GLFW_KEY_UP);
    bool downPressed = IsKeyPressed(GLFW_KEY_DOWN);

    // Increment bone index ONLY on press transition
    if (upPressed && !prevUpPressed)
    {
        currentBoneIndex++;
    }

    // Decrement bone index ONLY on press transition
    if (downPressed && !prevDownPressed)
    {
        currentBoneIndex--;
    }

    // Clamp to valid range [0, maxBoneIndex - 1]
    if (maxBoneIndex > 0)
    {
        currentBoneIndex = std::max(0, currentBoneIndex);
        currentBoneIndex = std::min(currentBoneIndex, maxBoneIndex - 1);
        printf("\n Current bone index %d!\n", currentBoneIndex);
    }

    // Store states for next frame
    prevUpPressed = upPressed;
    prevDownPressed = downPressed;

    // Vertical camera movement (Q / E) ----------------

    // Move camera up in world space
    if (IsKeyPressed(GLFW_KEY_E))
    {
        camera.MoveUp(speed);
    }

    // Move camera down in world space
    if (IsKeyPressed(GLFW_KEY_Q))
    {
        camera.MoveUp(-speed);
    }


    // Mouse drag handling ----------------

    double mouseX, mouseY;
    glfwGetCursorPos(gWindow, &mouseX, &mouseY);

    // Left mouse button pressed -> start dragging
    if (glfwGetMouseButton(gWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (!isDragging)
        {
            // First frame of drag: just store position
            isDragging = true;
            lastMouseX = mouseX;
            lastMouseY = mouseY;
        }
        else
        {
            // Compute mouse delta
            double dx = mouseX - lastMouseX;
            double dy = mouseY - lastMouseY;

            lastMouseX = mouseX;
            lastMouseY = mouseY;

            // Accumulate rotation angles
            modelYaw += (float)dx * mouseSensitivity;
            modelPitch += (float)dy * mouseSensitivity;

            // Optional: clamp pitch to avoid flipping
            const float limit = glm::radians(89.0f);
            modelPitch = glm::clamp(modelPitch, -limit, limit);
        }
    }
    else
    {
        // Mouse released
        isDragging = false;
    }
}

void InputController::SetMaxBoneIndex(int maxIndex)
{
    maxBoneIndex = std::max(0, maxIndex);
}

int InputController::GetCurrentBoneIndex() const
{
    return currentBoneIndex;
}

Camera& InputController::GetCamera()
{
    return camera;
}

glm::mat4 InputController::GetModelRotationMatrix() const
{
    glm::mat4 rot(1.0f);

    // Rotate around Y first (yaw), then X (pitch)
    rot = glm::rotate(rot, modelYaw,   glm::vec3(0, 1, 0));
    rot = glm::rotate(rot, modelPitch, glm::vec3(1, 0, 0));

    return rot;
}

