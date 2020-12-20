#include "VertexArray.hh"

namespace Render
{
	VertexArray::VertexArray(int count)
		:count(count)
	{
		glGenVertexArrays(1, (GLuint*)&vao);
	}

	VertexArray::VertexArray(float vertices[], byte indices[], float tCoords[])
	{
		count = *(&indices + 1) - indices;


		glGenVertexArrays(1, (GLuint*)&vao);
		glBindVertexArray(vao);

		glGenBuffers(1, (GLuint*)&vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, ((*(&vertices + 1) - vertices) * sizeof(float)), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(VERTEX_ATTRIB, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(VERTEX_ATTRIB);

		glGenBuffers(1, (GLuint*)&tbo);
		glBindBuffer(GL_ARRAY_BUFFER, tbo);
		glBufferData(GL_ARRAY_BUFFER, ((*(&tCoords + 1) - tCoords) * sizeof(byte)), tCoords, GL_STATIC_DRAW);
		glVertexAttribPointer(TCOORD_ATTRIB, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(TCOORD_ATTRIB);

		glGenBuffers(1, (GLuint*)&ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, ((*(&indices + 1) - indices) * sizeof(float)), indices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void VertexArray::Bind()
	{
		glBindVertexArray(vao);
		if(ibo > 0)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	}

	void VertexArray::Unbind()
	{
		if(ibo > 0)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void VertexArray::Draw()
	{
		if(ibo > 0)
			glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_BYTE, 0);
		else
			glDrawArrays(GL_TRIANGLES, 0, count);
	}
}