#version 330 core
layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;

out vec2 v_TexCoord;

uniform mat4 pr_matrix;

void main()
{
	gl_Position = pr_matrix * position;
	v_TexCoord = texCoord;
}
