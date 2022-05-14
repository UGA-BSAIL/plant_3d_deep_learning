#pragma once
#include "Renderer.h"

class Texture 
{
private:
	unsigned int textureId;
	std::string m_Filepath;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;

public:
	Texture(std::string& path);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetHeight() const { return m_Height; }
	inline int GetWidth() const { return m_Width; }
	
};