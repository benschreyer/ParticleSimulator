//Benjamin Schreyer 02/11/2021
#include "ComputeShader.h"


//Constructor that fetches and compiles shader at path
ComputeShader::ComputeShader(const std::string& filepath)
{
	std::ifstream t(filepath);
	std::stringstream tbuffer;
	tbuffer << t.rdbuf();
	std::cout << tbuffer.str() << "\n";
	t.close();
	std::string computeSource = tbuffer.str();
	
	GLCall(unsigned int computeId = glCreateShader(GL_COMPUTE_SHADER));
	const char* computeSrc = computeSource.c_str();
	GLCall(glShaderSource(computeId, 1, &computeSrc, nullptr));
	GLCall(glCompileShader(computeId));

	int computeCompileResult;

	GLCall(glGetShaderiv(computeId, GL_COMPILE_STATUS, &computeCompileResult));

	if (computeCompileResult == GL_FALSE)
	{
		int length;
		GLCall(glGetShaderiv(computeId, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)malloc(sizeof(char) * length);
		GLCall(glGetShaderInfoLog(computeId, length, &length, message));
		std::cout << "failed to compile shader\n" << message << '\n';
	}

	GLCall(unsigned int computeProgram = glCreateProgram());

	GLCall(glAttachShader(computeProgram, computeId));
	GLCall(glLinkProgram(computeProgram));
	GLCall(glValidateProgram(computeProgram));
	m_RendererID = computeProgram;
}
//Uniform setters
void ComputeShader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
	GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}
void ComputeShader::SetUniform1i(const std::string& name, int v0)
{
	GLCall(glUniform1i(GetUniformLocation(name), v0));
}
void ComputeShader::SetUniform1ui(const std::string& name, unsigned int v0)
{
	GLCall(glUniform1ui(GetUniformLocation(name), v0));
}
void ComputeShader::SetUniform1f(const std::string& name, float v0)
{
	GLCall(glUniform1f(GetUniformLocation(name), v0));
}

int ComputeShader::GetUniformLocation(const std::string& name)
{

	if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
		return m_UniformLocationCache[name];
	GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
	if (location == -1)
	{
		std::cout << name << " Does Not Exist\n";
	}
	std::cout << location << " EXISTS\n";
	m_UniformLocationCache[name] = location;
	return location;
}
ComputeShader::~ComputeShader()
{
	GLCall(glDeleteProgram(m_RendererID));
}
//Dispatce a compute call of a given size
void ComputeShader::Compute(unsigned int x,unsigned int y, unsigned int z) const
{
	for (int i = 0; i < this->buffers.size(); i++)
	{
		this->buffers[i]->bind();
	}
	GLCall(glUseProgram(m_RendererID));
	GLCall(glDispatchCompute(x, y, z));
	for (int i = 0; i < this->buffers.size(); i++)
	{
		this->buffers[i]->unbind();
	}
	//GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
}
//Add a buffer that is needed by the shader
void ComputeShader::addBuffer(BufferDescriptor* buf)
{
	this->buffers.push_back(buf);
}