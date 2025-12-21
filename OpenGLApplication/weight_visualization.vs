#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in ivec4 BoneIDs;
layout (location = 2) in vec4 Weights;

uniform mat4 MVP;

out vec4 vBoneIDs;
out vec4 vWeights;

void main()
{
    vBoneIDs = BoneIDs;
    vWeights = Weights;
    gl_Position = MVP * vec4(Position, 1.0);
}

// #version 330 core
// layout (location = 0) in vec3 Position;

// void main()
// {
//     gl_Position = vec4(Position.xy * 0.1, 0.0, 1.0);
// }
