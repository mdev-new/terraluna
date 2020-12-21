#include <fstream>
#include <iostream>
#include <sstream>

#include "Shader.hh"
#include "Misc/Maths/Matrix4f.hh"
#include "Misc/Maths/Vector2f.hh"
#include "Misc/Maths/Vector3f.hh"


namespace Shaders
{
	Shader::Shader() {}
	Shader::Shader(std::string& vertexData, std::string& fragmentData, bool onDisk)
	{
		std::string vertexShader, fragmentShader;

		if (onDisk)
		{
			auto vf = std::ifstream(vertexData, std::ios::in);
			auto fv = std::ifstream(fragmentData, std::ios::in);

			if (!vf)
				std::cout << "SHADER::LOAD_FROM_FILE::VERTEX_SHADER: Failed, could not open the file!\n";
			if (!fv)
				std::cout << "SHADER::LOAD_FROM_FILE::FRAGMENT_SHADER: Failed, could not open the file!\n";

			std::stringstream vs, fs;

			vs << vf.rdbuf();
			vf.close();

			fs << fv.rdbuf();
			fv.close();

			vertexShader = vs.str();
			fragmentShader = fs.str();

//			std::cout << "SHADER::LOAD_FROM_FILE::LOADED_SUCCESSFULLY: Loaded vertex & fragment shader from file successfully!" << '\n';
		}
		else
		{
			vertexShader = vertexData;
			fragmentShader = fragmentData;
		}

		uint32_t vertex, fragment;
		const char* vCode = vertexShader.c_str();
		const char* frCode = fragmentShader.c_str();
		
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vCode, NULL);
		glCompileShader(vertex);

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &frCode, NULL);
		glCompileShader(fragment);

		this->m_ProgramId = glCreateProgram();
		glAttachShader(this->m_ProgramId, vertex);
		glAttachShader(this->m_ProgramId, fragment);
		glLinkProgram(this->m_ProgramId);
		glValidateProgram(this->m_ProgramId);

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	Shader::~Shader()
	{
		glDeleteProgram(this->m_ProgramId);
	}

	void Shader::Bind()
	{
		glUseProgram(this->m_ProgramId);
	}

	void Shader::Unbind()
	{
		glUseProgram(0);
	}

	void Shader::SetUniform1i(std::string& name, int value)
	{
		glUniform1i(GetUniform(name), value);
	}

	void Shader::SetUniform1f(std::string& name, float value)
	{
		glUniform1f(GetUniform(name), value);
	}

	void Shader::SetUniform2f(std::string& name, Maths::Vector2f& vector)
	{
		glUniform2f(GetUniform(name), vector.x, vector.y);
	}

	void Shader::SetUniform3f(std::string& name, Maths::Vector3f& vector)
	{
		glUniform3f(GetUniform(name), vector.x, vector.y, vector.z);
	}

	void Shader::SetUniformMat4f(std::string& name, Maths::Matrix4f& matrix)
	{
		glUniformMatrix4fv(GetUniform(name), 1, GL_FALSE, matrix.elements);
	}

	// We can cache for improved speed
	int Shader::GetUniform(std::string& name)
	{
		int location = glGetUniformLocation(this->m_ProgramId, name.c_str());
		if(location == -1)
			std::cout << "Cant find uniform " << name << "!\n";

		return location;
	}
}
