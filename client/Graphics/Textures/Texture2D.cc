#include "Texture2D.hh"

namespace Textures
{
	Texture2D::Texture2D(std::string path)
	{
		int w;
		int h;
		int comp;
		unsigned char* image = stbi_load(path.c_str(), &w, &h, &comp, 3);

		if(image == nullptr || image == NULL)
			throw(std::string("Failed to load texture"));

		glGenTextures(1, (GLuint*)&id);
		glBindTexture(GL_TEXTURE_2D, id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if(comp == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, &image);
		else if(comp == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image);

		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(image);

		printf("texinfo: %s %i %i %i\n", path.c_str(), w, h, comp);
		width = w;
		height = h;
	}

	// RGBA
	Texture2D::Texture2D(int pixels[], int width, int height)
		:width(width), height(height)
	{
		int data[width * height];
		for (int i = 0; i < width * height; i++)
		{
			int a = (pixels[i] & 0xff000000) >> 24;
			int r = (pixels[i] & 0xff0000) >> 16;
			int g = (pixels[i] & 0xff00) >> 8;
			int b = (pixels[i] & 0xff);

			data[i] = a << 24 | b << 16 | g << 8 | r;
		}

		glGenTextures(1, (GLuint*)&id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture2D::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, id);
	}

	void Texture2D::Unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}
