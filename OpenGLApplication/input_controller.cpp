// Read GLFW key-states
// Updates camera and bone-index

#include "input_controller.h"
#include <GLFW/glfw3.h>
#include <algorithm>

extern GLFWwindow* gWindow;

InputController::InputController()
    : currentBoneIndex(0)
{
}

bool IsKeyPressed(int key)
{
    return glfwGetKey(gWindow, key) == GLFW_PRESS;
}

void InputController::Update(float deltaTime)
{
    printf("\n Pressed!\n");
    
    float speed = 5.0f * deltaTime;

    if (IsKeyPressed(GLFW_KEY_W)) camera.MoveForward(speed);
    if (IsKeyPressed(GLFW_KEY_S)) camera.MoveForward(-speed);
    if (IsKeyPressed(GLFW_KEY_A)) camera.MoveRight(-speed);
    if (IsKeyPressed(GLFW_KEY_D)) camera.MoveRight(speed);

    if (IsKeyPressed(GLFW_KEY_UP))   currentBoneIndex++;
    if (IsKeyPressed(GLFW_KEY_DOWN)) currentBoneIndex = std::max(0, currentBoneIndex - 1);
}

int InputController::GetCurrentBoneIndex() const
{
    return currentBoneIndex;
}

Camera& InputController::GetCamera()
{
    return camera;
}
