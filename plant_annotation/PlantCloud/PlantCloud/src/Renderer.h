#pragma once
#include <GL/glew.h>
#include "VertexBuffer.h"

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GlClearError();\
    x;\
    ASSERT(GlLogCall(#x, __FILE__, __LINE__));

void GlClearError();
bool GlLogCall(const char* function, const char* file, int line);



class Renderer 
{
private:
    float red;
    float red_increment;
public:
    Renderer();
    void Draw(const VertexArray& va, const  IndexBuffer& ib,  Shader& sh) ;
    void DrawLine(const VertexArray& va, const  IndexBuffer& ib, Shader& sh);
    void clear();
};
