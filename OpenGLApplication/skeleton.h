#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

/*
* Class that represents a skeleton while the program is running.
* 
* Will be traversed for skinning.
* 
* Needed since Assimp doesn't provide a skeleton, only the nodes
* where some of which are bones.
*/


// Represents a single bone in a skeleton
struct Bone
{
    std::string name;

    int parentIndex = -1;   // -1 means root bone

    // Assimp data
    glm::mat4 offsetMatrix{ 1.0f };      // Vertex-to-bone-space
    glm::mat4 localBindPose{ 1.0f };     // Node transform in bind pose, refrence pose

    // Runtime transforms - needed for animation
    glm::mat4 localPose{ 1.0f };         // Current local pose (anim & physics), will be overwritten every frame
    glm::mat4 globalPose{ 1.0f };        // Final world-space transform, computed outcome of the chain of offset-matrices, needed for skinning and constraints
};

// Represents a full skeleton hierarchy
struct Skeleton
{
    std::vector<Bone> bones;
    std::unordered_map<std::string, int> boneNameToIndex;   // Has same index/placement as in Main::vertex_to_bones
};