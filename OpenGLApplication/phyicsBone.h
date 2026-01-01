#pragma once
#include <glm/glm.hpp>

/*
* The following class/stucture represents bone-properties in a 
* format more appropriate for physics.Would have been used in 
* the Bullet integration.
*/

struct PhysicsBone
{
    int boneIndex = -1;        // Index into Skeleton::bones

    // Simulated transform (stand-in for physics)
    glm::vec3 position{ 0.0f };
    glm::quat rotation{ 1, 0, 0, 0 };

    // Parent index for constraints later
    int parentIndex = -1;
};