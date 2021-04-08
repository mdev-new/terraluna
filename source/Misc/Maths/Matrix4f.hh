#pragma once

#include "Vector3f.hh"

namespace Maths
{
	class Matrix4f
	{
	public:
		Matrix4f();
		Matrix4f Identity();
		Matrix4f Orthographic(float left, float right, float bottom, float top, float near, float far);
		Matrix4f Translate(Vector3f vector);
	 	Matrix4f Rotate(float angle);
		Matrix4f Multiply(Matrix4f matrix);

		float elements[4 * 4];

	private:
		int size = 4 * 4;
	};
}