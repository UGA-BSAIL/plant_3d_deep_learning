#pragma once

class IndexBuffer
{
private:

	unsigned int indexBufferId;
	unsigned int count;

public:
	IndexBuffer(const void* data, unsigned int count);
	~IndexBuffer();

	void Bind() const;
	void Unbind() const;

	inline unsigned int GetCount() const { return count; }
};

