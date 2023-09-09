#pragma once
class VertexBuffer 
{
private:

	unsigned int vertexBufferId;

public:
	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;
	void RebindBuffer(const void* data, unsigned int size);

};

