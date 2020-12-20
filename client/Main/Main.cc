#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <string>

#include "Main.hh"
#include "Graphics/Shaders/Shader.hh"


constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;

// Android compability
int main(int argc, char** argv)
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

	std::string fragShader = "shader.frag";
	std::string vertShader = "shader.vert";
	Shaders::Shader s(vertShader, fragShader, true);

	float vertices[] = {
		 0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
	};

	uint32_t indices[] = {
		0, 1, 2,
		1, 2, 3
	};

	uint32_t VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glClearColor(0.0f, 0.8f, 0.3f, 1.0f);

	while (!glfwWindowShouldClose(window))
	{
		// Update

		glClear(GL_COLOR_BUFFER_BIT);

		// Rendering
		s.Bind();

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		s.Unbind();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

	glfwTerminate();
	return 0;
}
