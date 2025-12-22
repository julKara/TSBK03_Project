#pragma once
#include <glm/glm.hpp>

struct PhysicsBone
{
    int boneIndex = -1;        // Index into Skeleton::bones

    // Simulated transform (stand-in for physics-engine)
    glm::vec3 position{ 0.0f };
    glm::quat rotation{ 1, 0, 0, 0 };

    // Cached parent index for constraints later
    int parentIndex = -1;
};