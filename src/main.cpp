#include "breakout/log.h"
#include "breakout/window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "breakout/renderer.h"
#include "breakout/resources.h"

int main(int argc, char** argv) {

	Window& window = Window::Get();
	window.Init(1200, 900, "Breakout");

	ShaderRef shader = Resources::TryGetShader("test", "res/shaders/basic_quad_shader");

	Renderer::SetShader(shader);

	float angle = 0.f;

	glClearColor(0.1f, 0.1f, 0.1f, 1.f);
	while (!window.ShouldClose()) {
		glClear(GL_COLOR_BUFFER_BIT);
		Renderer::Begin();


		Renderer::RenderQuad(glm::vec3( 0.f, 0.f, 0.f),		glm::vec2(0.1f), glm::vec4(1.f, 0.f, 0.f, 1.f));
		Renderer::RenderQuad(glm::vec3( 0.25f, 0.f, 0.f),	glm::vec2(0.1f), glm::vec4(0.f, 1.f, 0.f, 1.f));
		Renderer::RenderQuad(glm::vec3(-0.25f, 0.f, 0.f),	glm::vec2(0.1f), glm::vec4(0.f, 0.f, 1.f, 1.f));
		Renderer::RenderQuad(glm::vec3(0.0f, 0.25f, 0.f),	glm::vec2(0.1f), glm::vec4(1.f, .5f, 0.2f, 1.f));
		Renderer::RenderQuad(glm::vec3(0.0f,-0.25f, 0.f),	glm::vec2(0.1f), glm::vec4(1.f, 0.f, 1.f, 1.f));

		Renderer::RenderRotatedQuad(glm::vec3( 0.5f, 0.5f, 0.f), glm::vec2(0.1f), glm::radians( angle), glm::vec4(1.f, 1.f, 0.f, 1.f));
		Renderer::RenderRotatedQuad(glm::vec3(-0.5f,-0.5f, 0.f), glm::vec2(0.1f), glm::radians(-angle), glm::vec4(0.f, 1.f, 1.f, 1.f));

		angle += 2.f;
		if (angle > 360.f)
			angle -= 360.f;

		Renderer::End();
		window.SwapBuffers();
	}

	LOG(LOG_INFO, "Done.\n");
	return 0;
}