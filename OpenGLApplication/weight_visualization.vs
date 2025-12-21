#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in ivec4 BoneIDs;
layout (location = 3) in vec4 Weights;

uniform mat4 MVP;

out vec3 vNormal;
out vec4 vBoneIDs;
out vec4 vWeights;

void main()
{
    vNormal = normalize(Normal);
    vBoneIDs = vec4(BoneIDs);
    vWeights = Weights;

    gl_Position = MVP * vec4(Position, 1.0);
}