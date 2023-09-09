
#include<iostream>
#include "Renderer.h"

 void GlClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GlLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << " in " << function << "\nFile:" << file << ":" << line << ")" << std::endl;
        return false;
    }
    return true;
}

Renderer::Renderer()
    :red(0.8f), red_increment(0.05)
{
}

void Renderer::Draw(const VertexArray& va, const  IndexBuffer& ib,   Shader& sh) 
{
    // update red color
    if (red > 1.0f) red_increment = -0.05f;
    else if (red < 0.0f) red_increment = +0.05f;
    red += red_increment;

    // draw
    sh.Bind();
    //sh.SetUniform4f("u_Color", red, 0.1f, 0.3f, 1.0f);
    va.Bind();
    ib.Bind();
    GLCall(glDrawElements(GL_POINTS, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}


void Renderer::DrawLine(const VertexArray& va, const  IndexBuffer& ib, Shader& sh)
{
    // draw
    sh.Bind();
    //sh.SetUniform4f("u_Color", red, 0.1f, 0.3f, 1.0f);
    va.Bind();
    ib.Bind();
    GLCall(glDrawElements(GL_LINES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}


void Renderer::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
