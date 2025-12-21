#version 330 core

in vec4 vBoneIDs;
in vec4 vWeights;

uniform int uSelectedBone;

out vec4 FragColor;

// Maps weight to heat color
vec3 WeightToColor(float w)
{
    if (w > 0.66) return vec3(1.0, 0.0, 0.0); // red
    if (w > 0.33) return vec3(0.0, 1.0, 0.0); // green
    return vec3(0.0, 0.0, 1.0);               // blue
}

void main()
{
    float weight = 0.0;

    for (int i = 0; i < 4; i++)
    {
        if (int(vBoneIDs[i]) == uSelectedBone)
            weight = vWeights[i];
    }

    vec3 baseColor = vec3(0.0, 0.0, 0.3); // dark blue
    vec3 heatColor = WeightToColor(weight);

    FragColor = vec4(mix(baseColor, heatColor, weight), 1.0);

    //FragColor = vec4(0.0, 0.0, 1.0, 1.0); // solid blue
}
