#include "shader.h"
#include <glew.h>
#include <fstream>
#include <sstream>
#include <iostream>

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

Shader::Shader(const std::string& vsPath, const std::string& fsPath)
{
    std::string vsSrc = LoadFile(vsPath);
    std::string fsSrc = LoadFile(fsPath);

    if (vsSrc.empty() || fsSrc.empty()) {
        std::cerr << "Shader source empty\n";
        return;
    }

    const char* vsrc = vsSrc.c_str();
    const char* fsrc = fsSrc.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsrc, nullptr);
    glCompileShader(vs);

    GLint success;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(vs, 1024, nullptr, log);
        std::cerr << "VERTEX SHADER ERROR:\n" << log << std::endl;
        return;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsrc, nullptr);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(fs, 1024, nullptr, log);
        std::cerr << "FRAGMENT SHADER ERROR:\n" << log << std::endl;
        return;
    }

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(program, 1024, nullptr, log);
        std::cerr << "SHADER LINK ERROR:\n" << log << std::endl;
        return;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void Shader::Use() const
{
    //printf("\n Used!\n");
    glUseProgram(program);
}


void Shader::SetMat4Array(const std::string& name, const glm::mat4* data, int count)
{
    // Make sure this shader is active
    glUseProgram(program);

    GLint loc = glGetUniformLocation(program, name.c_str());
    if (loc == -1) return; // uniform not found (safe early-out)

    glUniformMatrix4fv(
        loc,
        count,
        GL_FALSE,
        &data[0][0][0]
    );
}


void Shader::SetMat4(const char* name, const glm::mat4& m) const
{
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, &m[0][0]);
}

void Shader::SetVec3(const char* name, const glm::vec3& v) const
{
    glUniform3fv(glGetUniformLocation(program, name), 1, &v[0]);
}

void Shader::SetInt(const char* name, int v) const
{
    glUniform1i(glGetUniformLocation(program, name), v);
}
