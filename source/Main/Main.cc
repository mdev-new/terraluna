#include <cstdio>
#include <string>
#include <vector>
#include <thread>
#include <sstream>

#include <iostream>

#include "Main.hh"
#include "Audio/Audio.hh"
#include "Misc/Maths/Matrix4f.hh"
#include "Graphics/Windows/Window.hh"
#include "Graphics/Render/VertexArray.hh"
#include "Graphics/Shaders/Shader.hh"
#include "Graphics/Textures/Texture2D.hh"

#include "Assets/VFS.hh"

bool keys [256];

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:
		keys[key] = true;
		break;
	case GLFW_RELEASE:
		keys[key] = false;
		break;
	}
}

void PrintDirs(Assets::CVFS &vfs, const std::string &Path, const std::string &Shift = "")
{
	auto node = vfs.GetNodeInfo(Path);
	auto childs = vfs.List(node);

	std::cout << Shift << "Dir: " << node->Name() << std::endl;
	for (auto &&i : childs)
	{
		if(i->IsDir())
			PrintDirs(vfs, Path + i->Name() + "/", Shift + " ");
		else
			std::cout << Shift << " File: " << i->Name() << " Size: " << vfs.FileSize(i) << std::endl;
	}
}

int main(void)
{
	{
		Assets::CVFS vfs;
		vfs.CreateDir("/data");

		auto fs = vfs.Open("/data/test.txt", Assets::FileMode::RW);
		fs->WriteLine("Hello World!");

		PrintDirs(vfs, "/");
		std::cout << fs->Read() << std::endl;
	}

	GLFWwindow *window;

	Graphics::MakeWindow(&window);
	glfwSetKeyCallback(window, key_callback);

	std::stringstream windowTitle;
	windowTitle << "Terraluna " << VERSION << " (OpenGL " << glGetString(GL_VERSION) << ")";
	glfwSetWindowTitle(window, windowTitle.str().c_str());

	glClearColor(0.0f, 0.8f, 0.3f, 1.0f);
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	std::string shader = "resources/shader.sdr";
	Graphics::Shader s(shader, true);

	std::string fname = "resources/vase.png";
	Graphics::Texture2D tex(fname);

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
	Graphics::VertexArray background(verts, indic, tcoords);

	Audio::SndOutStream snd;
	Audio::AudioFile af { "resources/test.mp3" };
	snd << af; // `snd.Play(af);` does the same thing
	// af.Wait(); // uncomment it to block the thread until the sound is played (efectively make this sync)

	bool running = true; // Can I question what's this?
	while (running && !glfwWindowShouldClose(window))
	{
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

		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FPS));
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}