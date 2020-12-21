#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <string>
#include <vector>

#include "Main.hh"
#include "Misc/Maths/Matrix4f.hh"
#include "Graphics/Render/VertexArray.hh"
#include "Graphics/Shaders/Shader.hh"
#include "Graphics/Textures/Texture2D.hh"


constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 800;

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

	glClearColor(0.0f, 0.8f, 0.3f, 1.0f);
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	std::string vertShader = "client/res/shader.vert";
	std::string fragShader = "client/res/shader.frag";
	Shaders::Shader s(vertShader, fragShader, true);

	std::string fname = "client/res/vase.png";
	Textures::Texture2D tex(fname);

	std::string prmat = "pr_matrix";
	std::string textext = "u_Texture";
	Maths::Matrix4f mat = Maths::Matrix4f().Orthographic(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1.0f, -1.0f);

	s.Bind();
	s.SetUniformMat4f(prmat, mat);
	s.SetUniform1i(textext, 1);
	s.Unbind();

	std::vector<float> verts = {
				0.0f, 300.0f, 0.1f,
				0.0f, 0.0f, 0.1f,
				300.0f, 0.0f, 0.1f,
				300.0f, 300.0f, 0.1f,
			};
	std::vector<float> tcoords = { 0, 1, 0, 0, 1, 0, 1, 1 };
	std::vector<byte> indic = { 0, 1, 2, 2, 3, 0 };
	Render::VertexArray background(verts, indic, tcoords);


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

// TODO: Cleanup
	glfwTerminate();
	return 0;
}
