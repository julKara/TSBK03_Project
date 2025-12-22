#pragma once
#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader(const std::string& vs, const std::string& fs);
    void Use() const;

    void SetMat4(const char* name, const glm::mat4& m) const;
    void SetVec3(const char* name, const glm::vec3& v) const;
    void SetInt(const char* name, int v) const;
    void SetMat4Array(const std::string& name, const glm::mat4* data, int count);


private:
    unsigned int program;
};
#pragma once
