#pragma once
#include <GL/glew.h>

#define __DEBUG

#define ASSERT(x) if (!(x)) __debugbreak();

#ifdef __DEBUG
#define GLCall(x) GLClearError(); x; ASSERT(GLLogCall())
#else
#define GLCall(x) x
#endif

bool GLLogCall();
void GLClearError();


inline bool GLLogCall()
{
	while (GLenum error = glGetError())
	{
		std::cout << "[OpenGL Error] (" << std::hex << error << ")\n";
		return false;
	}
	return true;
}

inline void GLClearError()
{
	while (glGetError() != GL_NO_ERROR); //returns 0 if no error
}