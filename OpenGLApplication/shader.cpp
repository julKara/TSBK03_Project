#include "shader.h"
#include <glew.h>
#include <fstream>
#include <sstream>

static std::string LoadFile(const std::string& path)
{
    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

Shader::Shader(const std::string& vsPath, const std::string& fsPath)
{
    std::string vsSrc = LoadFile(vsPath);
    std::string fsSrc = LoadFile(fsPath);

    const char* vsrc = vsSrc.c_str();
    const char* fsrc = fsSrc.c_str();

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsrc, nullptr);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsrc, nullptr);
    glCompileShader(fs);

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void Shader::Use() const
{
    glUseProgram(program);
}

void Shader::SetMat4(const char* name, const glm::mat4& m) const
{
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, &m[0][0]);
}

void Shader::SetInt(const char* name, int v) const
{
    glUniform1i(glGetUniformLocation(program, name), v);
}
