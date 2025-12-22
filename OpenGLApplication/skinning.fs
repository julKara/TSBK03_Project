#version 330 core

in vec3 vNormal;
out vec4 FragColor;

void main()
{
    vec3 n = normalize(vNormal);
    float lighting = dot(n, normalize(vec3(0.3, 0.7, 0.6)));
    lighting = clamp(lighting, 0.2, 1.0);

    FragColor = vec4(vec3(lighting), 1.0);
}