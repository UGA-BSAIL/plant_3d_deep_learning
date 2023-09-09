

#pragma once
#include "VertexArray.h"
#include "VertexBufferLayout.h"

VertexArray::VertexArray()
{
	GLCall(glGenVertexArrays(1, &vertexArrayId));
}

VertexArray::~VertexArray()
{
	GLCall(glDeleteVertexArrays(1, &vertexArrayId));

}

void VertexArray::Bind() const
{
	GLCall(glBindVertexArray( vertexArrayId));
}

void VertexArray::Unbind() const
{
	GLCall(glBindVertexArray( 0));
}


void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
	Bind();
	vb.Bind();
	const auto& elements = layout.GetElements();
	unsigned int offset = 0;
	for (unsigned i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), (const void*)offset));
		GLCall(glEnableVertexAttribArray(i));
		offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
	}
}
