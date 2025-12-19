// Stores camera-state
// Provides movement-fucntions

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera();

    void MoveForward(float delta);
    void MoveRight(float delta);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspect) const;

    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;

private:
    float Fov;
};

