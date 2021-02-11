#pragma once
#include "Debug.h"


class BufferDescriptor
{
public:
	GLuint m_RendererID;
	GLenum target;
	GLuint position;

	BufferDescriptor(GLuint m_RendererID, GLenum target, GLuint position);


	void bind();
	
	void unbind();

};

inline BufferDescriptor::BufferDescriptor(GLuint m_RendererID, GLenum target, GLuint position)
{
	this->target = target;
	this->m_RendererID = m_RendererID;
	this->position = position;
}


inline void BufferDescriptor::bind()
{
	GLCall(glBindBufferBase(this->target, this->position, this->m_RendererID));
}


inline void BufferDescriptor::unbind()
{
	GLCall(glBindBufferBase(this->target, this->position, NULL));
}


