#pragma once

#include <string>

#include "Misc/Maths/Matrix4f.hh"
#include "Misc/Maths/Vector2f.hh"
#include "Misc/Maths/Vector3f.hh"


namespace Shaders
{
	class Shader
	{
	public:
		Shader();

		/*
			Shader(std::string& vertexData, std::string& fragmentData, bool onDisk)
			vertexData - the shader, or path in the disk (onDisk has to be true if on disk)
			fragmentData - the shader, or path in the disk (onDisk has to be tru if on disk)
			onDisk - load data from files.
		*/
		Shader(std::string& shaderData, bool onDisk);

		~Shader();

		void Bind();
		void Unbind();

		void SetUniform1i(std::string& name, int value);
		void SetUniform1f(std::string& name, float value);
		void SetUniform2f(std::string& name, Maths::Vector2f& vector);
		void SetUniform3f(std::string& name, Maths::Vector3f& vector);
		void SetUniformMat4f(std::string& name, Maths::Matrix4f& matrix);
		int GetUniform(std::string& name);
	private:
		uint32_t m_ProgramId;
	};
}
