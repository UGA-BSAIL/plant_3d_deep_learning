#pragma once

#include "VertexBuffer.h"
class VertexBufferLayout;

class VertexArray {

private:
	unsigned int vertexArrayId;

public:
	VertexArray();
	~VertexArray();

	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
	void Bind() const;
	void Unbind() const;

};

