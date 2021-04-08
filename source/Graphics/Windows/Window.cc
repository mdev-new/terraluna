#include "Window.hh"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

int Windows::MakeWindow(GLFWwindow **window) {
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	*window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Terraluna", NULL, NULL);

	if (!(*window))
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(*window);
	glfwSwapInterval(0);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		glfwTerminate();
		return -1;
	}

	return 0;
}