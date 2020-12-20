#pragma once

#include "Vector3f.hh"

namespace Maths
{
	class Matrix4f
	{
	public:
		Matrix4f();
		static Matrix4f Identity();
		static Matrix4f Orthographic(float left, float right, float bottom, float top, float near, float far);
		static Matrix4f Translate(Vector3f vector);
		static Matrix4f Rotate(float angle);
		Matrix4f Multiply(Matrix4f matrix);

		float elements[];

	private:
		int size = 4 * 4;
	};
}
