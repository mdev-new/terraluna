#pragma once
typedef char byte;

#include <glad/glad.h>

#include "Main/Main.hh"

namespace Render
{
	class VertexArray
	{
	public:
		VertexArray(int count);
		VertexArray(float vertices[], byte indices[], float tCoords[]);
		void Bind();
		void Unbind();
		void Draw();

	private:
		int vao, vbo, ibo, tbo;
		int count;
	};
}
