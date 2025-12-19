#pragma once
#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader(const std::string& vs, const std::string& fs);
    void Use() const;

    void SetMat4(const char* name, const glm::mat4& m) const;
    void SetInt(const char* name, int v) const;

private:
    unsigned int program;
};
#pragma once
