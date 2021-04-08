#include "Shader.hh"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>

#include <glad.h>

namespace Shaders
{
	Shader::Shader() {}
	Shader::Shader(std::string& shaderData, bool onDisk)
	{
		std::string vertexShader, fragmentShader;

		if (onDisk)
		{
			std::ifstream shader(shaderData, std::ios::in);

			std::string line;
			std::stringstream ss[2];
			int type = 0; // -1 - none, 0 - vertex, 1 - fragment

			while (getline(shader, line))
			{
				if(line.find("#type") != std::string::npos)
				{
					if (line.find("vertex") != std::string::npos)
						type = 0;
					else if(line.find("fragment") != std::string::npos)
						type = 1;
				}
				else
				{
					ss[type] << line << '\n';
				}
			}

			shader.close();
			vertexShader = ss[0].str();
			fragmentShader = ss[1].str();
		}
		else
		{
			std::istringstream lines(shaderData);
			std::stringstream ss[2];
			std::string line;
			int type = 0; // -1 - none, 0 - vertex, 1 - fragment
			while (getline(lines, line))
			{
				if(line.find("#type") != std::string::npos)
				{
					if (line.find("vertex") != std::string::npos)
						type = 0;
					else if(line.find("fragment") != std::string::npos)
						type = 1;
				}
				else
				{
					ss[type] << line << "\n";
				}
			}

			vertexShader = ss[0].str();
			fragmentShader = ss[1].str();
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