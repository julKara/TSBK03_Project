#version 330 core

in vec3 vNormal;
in vec4 vBoneIDs;
in vec4 vWeights;

uniform int uSelectedBone;
uniform vec3 uLightDir;

out vec4 FragColor;

// Maps weight to heat color
vec3 WeightToColor(float w)
{
    if (w >= 0.7) return vec3(1.0, 0.0, 0.0); // red
    if (w >= 0.4 && w < 0.7) return vec3(0.0, 1.0, 0.0); // green
    if (w > 0.1) return vec3(1.0, 1.0, 0.0); // yellow
    return vec3(0.0, 0.0, 1.0);               // blue
}

void main()
{
    // Find weight of selected bone
    float weight = 0.0;
    for (int i = 0; i < 4; i++)
    {
        if (int(vBoneIDs[i]) == uSelectedBone)
            weight = vWeights[i];
    }

    // Base color from bone weight
    vec3 baseColor = mix(
        vec3(0.0588, 0.0588, 0.4118),   // dark blue base color
        WeightToColor(weight),
        weight
    );

    // Simple diffuse lighting
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);

    float diffuse = max(dot(N, L), 0.0);

    // Add small ambient so backfaces are visible
    float ambient = 0.25;

    vec3 finalColor = baseColor * (ambient + diffuse);

    FragColor = vec4(finalColor, 1.0);
}