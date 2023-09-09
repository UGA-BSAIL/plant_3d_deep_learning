#pragma once
#include <iostream>
#include <unordered_map>
#include "glm\glm.hpp"


class Shader 
{
private:
	unsigned int shaderId;	
	std::string filepath;
	std::unordered_map<std::string, int > m_UniformLocationCache;


public:
	Shader(std::string& filepath);
	~Shader();

	void Bind() const;
	void Unbind() const;

	void SetUniform4f(const std::string& name, float f0, float f1, float f2, float f3) ;
	void SetUniform1i(const std::string& name, int value);
	void SetUniform4fMat(const std::string& name, glm::mat4& matrix);

	int CompileShader(unsigned int type, const std::string& source);
	int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);

private:
	int GetUniformLocation(const std::string& name) ;
};
