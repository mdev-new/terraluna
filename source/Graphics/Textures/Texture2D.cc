#include "Texture2D.hh"

#include <iostream>
#include <glad/glad.h>
#include <stb_image.h>

namespace Textures
{
	Texture2D::Texture2D(std::string& path)
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		stbi_set_flip_vertically_on_load(false);
		data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		if (data)
		{
			if(channels == 3)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			else if(channels == 4)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			printf("Failed to load texture\n");
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);
	}

	void Texture2D::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	void Texture2D::Unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}
