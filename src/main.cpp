#include "breakout/log.h"
#include "breakout/window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "breakout/gameobject.h"
#include "breakout/resources.h"

int main(int argc, char** argv) {

	Window& window = Window::Get();
	window.Init(1200, 900, "Breakout");

	ShaderRef shader = Resources::TryGetShader("test", "res/shaders/tst");

	glClearColor(0.1f, 0.1f, 0.1f, 1.f);
	while (!window.ShouldClose()) {
		glClear(GL_COLOR_BUFFER_BIT);



		window.SwapBuffers();
	}

	LOG(LOG_INFO, "Done.\n");
	return 0;
}