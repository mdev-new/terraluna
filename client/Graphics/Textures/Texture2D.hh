#pragma once
#include <string>
#include <glad/glad.h>

#include <stb_image.h>

namespace Textures
{
	class Texture2D
	{
	public:
		Texture2D(std::string path);
		Texture2D(int pixels[], int width, int height);
		void Bind();
		void Unbind();


	private:
		GLuint width, height;
		GLuint id;
	};
}
