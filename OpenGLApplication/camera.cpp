// Implements camera math and movement

#include "camera.h"

Camera::Camera()
{
    Position = glm::vec3(0.0f, 0.0f, 5.0f);
    Front = glm::vec3(0.0f, 0.0f, -1.0f);
    Up = glm::vec3(0.0f, 1.0f, 0.0f);
    Fov = 45.0f;
}

void Camera::MoveForward(float delta)
{
    Position += Front * delta;
}

void Camera::MoveRight(float delta)
{
    Position += glm::normalize(glm::cross(Front, Up)) * delta;
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspect) const
{
    return glm::perspective(glm::radians(Fov), aspect, 0.1f, 100.0f);
}
