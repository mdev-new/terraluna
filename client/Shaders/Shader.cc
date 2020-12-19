#include "Shader.hh"
#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <sstream>

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

			std::cout << "SHADER::LOAD_FROM_FILE::LOADED_SUCCESSFULLY: Loaded vertex & fragment shader from file successfully!" << '\n';
		}
		else
		{
			vertexShader = vertexData;
			fragmentShader = fragmentData;
		}

		uint32_t vertex, fragment;
		const char* vCode = vertexShader.c_str();
		const char* fCode = fragmentShader.c_str();
		
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vCode, NULL);
		glCompileShader(vertex);

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fCode, NULL);
		glCompileShader(fragment);

		this->m_ProgramId = glCreateProgram();
        glAttachShader(this->m_ProgramId, vertex);
        glAttachShader(this->m_ProgramId, fragment);
        glLinkProgram(this->m_ProgramId);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
	}

	void Shader::UseProgram() {}

	template<typename T>
	void Shader::SetT(std::string& name, T value) {}
}