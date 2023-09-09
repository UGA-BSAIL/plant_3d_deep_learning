
#pragma once
#include "Renderer.h"
#include "Shader.h"
#include<fstream>
#include<sstream>

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

ShaderProgramSource ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;

    while (getline(stream, line))
    {
        if (line.find("shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos) { type = ShaderType::VERTEX; }
            else if (line.find("fragment") != std::string::npos) { type = ShaderType::FRAGMENT; }
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }
    return { ss[0].str() , ss[1].str() };

}

int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    // Error handling

    int result;

    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)alloca(length * sizeof(char));
        GLCall(glGetShaderInfoLog(id, length, &length, message));

        std::cout << "Failed tp compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
            << " shader" << std::endl;
        std::cout << message << std::endl;
        GLCall(glDeleteShader(id));
        return 0;
    }

    return id;
}
int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

Shader::Shader(std::string& filepath)
	: filepath(filepath), shaderId(0)
{
    ShaderProgramSource source = ParseShader(filepath);
    shaderId = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
	GLCall(glDeleteProgram(shaderId));

}

void Shader::Bind() const
{
	GLCall(glUseProgram(shaderId));
}

void Shader::Unbind() const
{
	GLCall(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    GLCall(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniform4fMat(const std::string& name, glm::mat4& matrix)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::SetUniform4f(const std::string& name, float f0, float f1, float f2, float f3) 
{
    GLCall(glUniform4f(GetUniformLocation(name), f0, f1, f2, f3));
}

int Shader::GetUniformLocation(const std::string& name) 
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    GLCall(int location = glGetUniformLocation(shaderId, name.c_str()));
    if (location == -1) 
    {
        std::cout << "Note: Uniform does not exist. Uniform name: " << name << std:: endl;
    }
    return location;
}
