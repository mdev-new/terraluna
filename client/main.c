#include <glad/glad.h>
#include <GLFW/glfw3.h>
// Dimensions of our window
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
int main(void)
{
	GLFWwindow* window;

	if ( ! glfwInit() )
	{
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL: Starting point", NULL, NULL);

	if ( ! window )
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if ( ! gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) )
	{
		glfwTerminate();
		return -1;
	}

	while ( ! glfwWindowShouldClose(window) )
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}
