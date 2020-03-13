#include "Renderer.h"

#include <iostream>
bool GLLogCall()
{
	while (GLenum error = glGetError())
	{
		std::cout << "[OpenGL Error] (" << std::hex << error << ")\n";
		return false;
	}
	return true;
}

void GLClearError()
{
	while (glGetError() != GL_NO_ERROR); //returns 0 if no error
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const
{
	ib.Bind();
	va.Bind();
	shader.Bind();
	GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(),GL_UNSIGNED_INT,nullptr ));
}
void Renderer::Clear()
{
	
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}
