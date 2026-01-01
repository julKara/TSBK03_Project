#include "shader.h"
#include <glew.h>
#include <fstream>
#include <sstream>
#include <iostream>

/*
* Class whose purpose is to simplify using/switching
* between multible shaders.
*
* Important since project contains multiple shader used exclusivly
* for debugging such as weight-visualization and bone-lines. Also enables
* the use of multible shader at the same time, you can visualize the bone as lines
* while rendering the mesh "normally" to see the movement.
*/

// Basic file-reader
static std::string LoadFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "FAILED TO OPEN SHADER FILE: " << path << std::endl;
        return "";
    }

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Constructor - loads the shader from disk (located within the same src-folder)
Shader::Shader(const std::string& vsPath, const std::string& fsPath)
{
    // Load vertex- and fragment.shader by name
    std::string vsSrc = LoadFile(vsPath);
    std::string fsSrc = LoadFile(fsPath);

    // Test file
    if (vsSrc.empty() || fsSrc.empty()) {
        std::cerr << "Shader source empty\n";
        return;
    }

    const char* vsrc = vsSrc.c_str();
    const char* fsrc = fsSrc.c_str();

    // Create and compile vertex-shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsrc, nullptr);
    glCompileShader(vs);

    // Test if vertex-shader compiled
    GLint success;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(vs, 1024, nullptr, log);
        std::cerr << "VERTEX SHADER ERROR:\n" << log << std::endl;
        return;
    }


    // Create and compile fragment-shader
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsrc, nullptr);
    glCompileShader(fs);

    // Test if fragment-shader compiled
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(fs, 1024, nullptr, log);
        std::cerr << "FRAGMENT SHADER ERROR:\n" << log << std::endl;
        return;
    }

    // Create and link program
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);


    // Test if program compiled
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(program, 1024, nullptr, log);
        std::cerr << "SHADER LINK ERROR:\n" << log << std::endl;
        return;
    }

    // Delete shaders after succesful compilation so the they can be refilled
    glDeleteShader(vs);
    glDeleteShader(fs);
}

// Makes program us this program (shader).
void Shader::Use() const
{
    //printf("\n Used!\n");
    glUseProgram(program);
}

// ------------------------- UNIFORM ATTRIBUTES -------------------------

// Upload array of matrices such as bone-matrices
void Shader::SetMat4Array(const std::string& name, const glm::mat4* data, int count)
{
    // Make sure this shader is active
    glUseProgram(program);

    GLint loc = glGetUniformLocation(program, name.c_str());
    if (loc == -1) return; // return if uniform not found

    glUniformMatrix4fv(loc, count, GL_FALSE, &data[0][0][0]);
}

// Upload a single matrix such projection-matrix
void Shader::SetMat4(const char* name, const glm::mat4& m) const
{
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, &m[0][0]);
}

// Upload a single vector such as veiw-position
void Shader::SetVec3(const char* name, const glm::vec3& v) const
{
    glUniform3fv(glGetUniformLocation(program, name), 1, &v[0]);
}

// Upload a single int value such as the currently selected bone in weight-viz
void Shader::SetInt(const char* name, int v) const
{
    glUniform1i(glGetUniformLocation(program, name), v);
}
