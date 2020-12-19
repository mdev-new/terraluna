#pragma once
#include <glad/glad.h>
#include <string>
#include <cstdint>


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
		Shader(std::string& vertexData, std::string& fragmentData, bool onDisk);

		void UseProgram();

		template<typename T>
		void SetT(std::string& name, T value);
	private:
		uint32_t m_ProgramId;
	};

}
