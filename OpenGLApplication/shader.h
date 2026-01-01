#pragma once
#include <string>
#include <glm/glm.hpp>

/*
* Class whose purpose is to simplify using/switching
* between multible shaders. 
* 
* Important since project contains multiple shader used exclusivly
* for debugging such as weight-visualization and bone-lines. Also enables
* the use of multible shader at the same time, you can visualize the bone as lines
* while rendering the mesh "normally" to see the movement.
*/

class Shader
{
public:

    // Contructor
    Shader(const std::string& vs, const std::string& fs);
    
    // Makes the program use this program (shader).
    void Use() const;

    // Load up uniform attributes
    void SetMat4(const char* name, const glm::mat4& m) const;
    void SetVec3(const char* name, const glm::vec3& v) const;
    void SetInt(const char* name, int v) const;
    void SetMat4Array(const std::string& name, const glm::mat4* data, int count);


private:
    unsigned int program;
};
