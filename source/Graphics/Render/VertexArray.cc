#include "VertexArray.hh"
#include <cstdio>

namespace Render
{
	VertexArray::VertexArray(int count)
		:count(count)
	{
		glGenVertexArrays(1, &vao);
	}

	VertexArray::VertexArray(std::vector<float> vertices, std::vector<byte> indices, std::vector<float> tCoords)
		:vertices(vertices), tCoords(tCoords), indices(indices)
	{
		count = indices.size();

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices.front(), GL_STATIC_DRAW);
		glVertexAttribPointer(VERTEX_ATTRIB, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &tbo);
		glBindBuffer(GL_ARRAY_BUFFER, tbo);
		glBufferData(GL_ARRAY_BUFFER, tCoords.size() * sizeof(float), &tCoords.front(), GL_STATIC_DRAW);
		glVertexAttribPointer(TCOORD_ATTRIB, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(byte), &indices.front(), GL_STATIC_DRAW);


//		glVertexAttribPointer(glGetAttribLocation(program, "point"), 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
//		glVertexAttribPointer(glGetAttribLocation(program, "texcoord"), 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

		glEnableVertexAttribArray(VERTEX_ATTRIB);
		glEnableVertexAttribArray(TCOORD_ATTRIB);

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
			glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_BYTE, (void*)0);
		else
			glDrawArrays(GL_TRIANGLES, 0, count);
	}
}