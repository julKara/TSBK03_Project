#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/*
* Very basic camera implementation, needed for having
* interaction which is needed for differing models.
*/

class Camera
{
public:
    Camera();

    // Movement functions
    void MoveForward(float delta);
    void MoveRight(float delta);
    void MoveUp(float amount);

    // Normal camera attributes
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspect) const;

    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;

private:
    float Fov;
};

