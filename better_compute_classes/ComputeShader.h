//Benjamin Schreyer 02/11/2021
#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iostream>
#include <fstream>
#include "BufferDescriptor.h"
#include "Debug.h"

//Quickly setup and compile compute shaders
class ComputeShader
{
public:
	ComputeShader(const std::string& src);
	~ComputeShader();

	void addBuffer(BufferDescriptor* buf);

	void Compute(unsigned int x, unsigned int y, unsigned int z) const;
	//void Unbind() const;
	int GetUniformLocation(const std::string& name);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniform1i(const std::string& name, int v0);
	void SetUniform1ui(const std::string& name, unsigned int v0);
	void SetUniform1f(const std::string& name, float v0);
	unsigned int m_RendererID;

	
	std::unordered_map<std::string, int> m_UniformLocationCache;
	//std::vector<unsigned int> SSBOID;
	//std::vector<unsigned int> SSBOPOS;

	//Buffers that need to be bound for this shader
	std::vector<BufferDescriptor*> buffers;
};