#pragma once
#include "IndexBuffer.h"
#include "Renderer.h"

IndexBuffer::IndexBuffer(const void* data, unsigned int count)
    : count(count)
{
    ASSERT( sizeof(unsigned int) == sizeof(GLuint) );
    GLCall(glGenBuffers(1, &indexBufferId));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer()
{
    GLCall(glDeleteBuffers(1, &indexBufferId));
}

void IndexBuffer::Bind() const
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId));
}

void IndexBuffer::Unbind() const
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
