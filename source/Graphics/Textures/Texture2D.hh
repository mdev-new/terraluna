#pragma once
#include <string>

namespace Graphics
{
	class Texture2D
	{
	public:
		Texture2D(std::string& path);
		Texture2D(int pixels[], int width, int height);
		void Bind();
		void Unbind();

	private:
		int width, height, channels;
		unsigned char* data;
		unsigned int texture;
	};
}