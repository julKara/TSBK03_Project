#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

// Represents a single bone in a skeleton
struct Bone
{
    std::string name;
    int parentIndex = -1;           // -1 = root
    glm::mat4 offsetMatrix{ 1.0f };   // Assimp offset matrix
    glm::mat4 localBindPose{ 1.0f };  // Node transform
};

// Represents a full skeleton hierarchy
struct Skeleton
{
    std::vector<Bone> bones;
    std::unordered_map<std::string, int> boneNameToIndex;
};