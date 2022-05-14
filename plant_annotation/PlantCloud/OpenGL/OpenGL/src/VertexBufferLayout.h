#pragma once
#include<vector>
#include "Renderer.h"

struct VertexBufferElement {
	unsigned int type;
	unsigned int count;
	unsigned char normalized;


	static unsigned int GetSizeOfType(unsigned int type) {
		switch (type) 
		{
		case GL_FLOAT: return 4;
		case GL_UNSIGNED_INT: return 4;
		case GL_UNSIGNED_BYTE: return 1;
		}
		ASSERT(false);
		return 0;
	}
};

class VertexBufferLayout 
{
private:
	std::vector<VertexBufferElement> m_Elements;
	unsigned int m_Stride;
public:
	VertexBufferLayout()
		:m_Stride(0) {}

	template<typename T>
	void Push(unsigned int count)
	{
		static_assert(false);
	}

	template<>
	void Push<float>(unsigned int count)
	{
		VertexBufferElement myelem = { GL_FLOAT, count, GL_FALSE };
		m_Elements.push_back(myelem);
		m_Stride += count*VertexBufferElement::GetSizeOfType(GL_FLOAT);//sizeof(GLfloat);
	}

	template<>
	void Push<unsigned int>(unsigned int count)
	{
		VertexBufferElement myelem = { GL_UNSIGNED_INT, count, GL_FALSE };
		m_Elements.push_back(myelem);
		m_Stride += count* VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT); // sizeof(GLuint);
	}
	
	inline const std::vector<VertexBufferElement> GetElements() const { return m_Elements; }
	inline unsigned int GetStride() const { return m_Stride; }

};
