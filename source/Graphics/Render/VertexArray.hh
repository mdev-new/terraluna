#pragma once

#include <vector>
#include <glad.h>

#include "Main/Main.hh"

typedef GLubyte byte;
namespace Render
{
	class VertexArray
	{
	public:
		VertexArray(int count);
		VertexArray(std::vector<float> vertices, std::vector<byte> indices, std::vector<float> tCoords);
		void Bind();
		void Unbind();
		void Draw();

	private:
		GLuint vao, vbo, ibo, tbo;
		int count;
		std::vector<float> vertices;
		std::vector<float> tCoords;
		std::vector<byte> indices;
	};
}