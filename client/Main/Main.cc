#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <string>

#define STB_IMAGE_IMPLEMENTATION

#include "Main.hh"
#include "Misc/Maths/Matrix4f.hh"
#include "Graphics/Render/VertexArray.hh"
#include "Graphics/Shaders/Shader.hh"
#include "Graphics/Textures/Texture2D.hh"


constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;

// Android compability
int main(/*int argc, char** argv*/)
{
	GLFWwindow* window;
	
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Terraluna", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		glfwTerminate();
		return -1;
	}
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	//glClearColor(0.0f, 0.8f, 0.3f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	std::string fragShader = "shader2.frag";
	std::string vertShader = "shader2.vert";
	Shaders::Shader s(vertShader, fragShader, true);

	std::string unif = "pr_matrix";
	Maths::Matrix4f mat = Maths::Matrix4f().Orthographic(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1.0f, 1.0f);
	s.SetUniformMat4f(unif, mat);
	std::string textext = "tex";
	s.SetUniform1i(textext, 1);
	float verts[] = { 0.0f, SCREEN_WIDTH, 0.0f, 0.0f, 0.0f, 0.0f, SCREEN_WIDTH, 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, };
	float tcoords[] = { 0, 1, 0, 0, 1, 0, 1, 1 };
	byte indic[] = { 0, 1, 2, 2, 3, 0 };
	Render::VertexArray background(verts, indic, tcoords);


	Textures::Texture2D tex("texture.png");

	while (!glfwWindowShouldClose(window))
	{
		// Update


		// Rendering
		glClear(GL_COLOR_BUFFER_BIT);
		tex.Bind();
		s.Bind();
		background.Bind();
		background.Draw();
		s.Unbind();
		tex.Unbind();
		background.Unbind();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
