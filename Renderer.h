#pragma once
#include<GL/glew.h>
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#define __DEBUG

#define ASSERT(x) if (!(x)) __debugbreak();

#ifdef __DEBUG
#define GLCall(x) GLClearError(); x; ASSERT(GLLogCall())
#else
#define GLCall(x) x
#endif

bool GLLogCall();
void GLClearError();

class Renderer
{
public:
	void Clear();
	void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
};