#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in ivec4 aBoneIDs;
layout (location = 3) in vec4 aWeights;

uniform mat4 MVP;

// Must match your uploadBoneMatrices clamp
uniform mat4 uBones[128];

out vec3 vNormal;

void main()
{
    mat4 skinMatrix =
          aWeights.x * uBones[aBoneIDs.x]
        + aWeights.y * uBones[aBoneIDs.y]
        + aWeights.z * uBones[aBoneIDs.z]
        + aWeights.w * uBones[aBoneIDs.w];

    vec4 skinnedPosition = skinMatrix * vec4(aPosition, 1.0);

    gl_Position = MVP * skinnedPosition;

    // Transform normal (simple version) - can be removed
    vNormal = mat3(skinMatrix) * aNormal;
}