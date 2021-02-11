#pragma once
#include "BufferDescriptor.h"
#include "Debug.h"

template <typename T>
class ShaderStorageBufferObject
{
public:
	ShaderStorageBufferObject<T>(int length, GLbitfield access, GLbitfield mapAccess, GLenum usage, GLuint position);
	~ShaderStorageBufferObject();

	T* getMap();
	
	GLuint m_RendererID;

	void bind();

	void unbind();


	GLbitfield mapAccess;
	GLbitfield access;
	GLenum usage;
	int length;
	GLuint position;
	T* map = NULL;

	BufferDescriptor* bufferDescriptor;

	BufferDescriptor* getDescriptor();
	

};

template <typename T>

inline ShaderStorageBufferObject<T>::ShaderStorageBufferObject(int length, GLbitfield access, GLbitfield mapAccess, GLenum usage,GLuint position)
{
	this->mapAccess = mapAccess;
	this->position = position;
	this->access = access;
	this->length = length;
	this->usage = usage;

	

	GLCall(glGenBuffers(1, &this->m_RendererID));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->m_RendererID));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, this->length * sizeof(T), NULL, usage));

	this->bufferDescriptor = new BufferDescriptor(this->m_RendererID, GL_SHADER_STORAGE_BUFFER, this->position);
}

template <typename T>

inline ShaderStorageBufferObject<T>::~ShaderStorageBufferObject()
{
	GLCall(glUnmapNamedBuffer(this->m_RendererID));
	GLCall(glDeleteBuffers(1, &this->m_RendereID));
}

template <typename T>

inline T* ShaderStorageBufferObject<T>::getMap()
{
	
	if (this->map == NULL)
	{

		this->bind();
		GLCall(this->map = (T*)glMapNamedBufferRange(this->m_RendererID,0,this->length, this->mapAccess));
		this->unbind();
		return this->map;
	}
	return this->map;


}
template <typename T>

inline void ShaderStorageBufferObject<T>::bind()
{
	GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER,this->position, this->m_RendererID));
}

template <typename T>

inline void ShaderStorageBufferObject<T>::unbind()
{
	GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, this->position, NULL));
}

template <typename T>

inline BufferDescriptor* ShaderStorageBufferObject<T>::getDescriptor()
{
	return this->bufferDescriptor;
}