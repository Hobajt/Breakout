#include "breakout/log.h"
#include "breakout/window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "breakout/renderer.h"
#include "breakout/resources.h"
#include "breakout/texture.h"

int main(int argc, char** argv) {

	Window& window = Window::Get();
	window.Init(1200, 900, "Breakout");

	ShaderRef shader = Resources::TryGetShader("test", "res/shaders/basic_quad_shader");

	ITextureRef texture1 = Resources::TryGetTexture("test", "res/textures/test.png");
	ITextureRef texture2 = Resources::TryGetTexture("lena", "res/textures/lena.png");

	AtlasTexture atlas = AtlasTexture("res/textures/test_atlas.png", glm::ivec2(128, 128));

	ITextureRef texture3 = atlas(2, 3);

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

		Renderer::RenderQuad(glm::vec3(0.25f, -0.25f, 0.f), glm::vec2(0.1f), texture1);
		Renderer::RenderRotatedQuad(glm::vec3(0.5f, -0.5f, 0.f), glm::vec2(0.1f), glm::radians(-angle), texture1);

		Renderer::RenderQuad(glm::vec3(-0.25f, 0.25f, 0.f), glm::vec2(0.1f), texture2);
		Renderer::RenderRotatedQuad(glm::vec3(-0.5f, 0.5f, 0.f), glm::vec2(0.1f), glm::radians(-angle), texture2);

		for (int x = 0; x < 5; x++) {
			for (int y = 0; y < 5; y++) {
				Renderer::RenderQuad(glm::vec3(-0.25 + 0.11*x, -0.4 - 0.11*y, 0.f), glm::vec2(0.05f), atlas(7+x, 6+y));
			}

		}

		//Renderer::RenderQuad(glm::vec3(-0.25, -0.75, 0.f), glm::vec2(0.05f), atlas(1, 0));
		//Renderer::RenderQuad(glm::vec3(-0.15, -0.75, 0.f), glm::vec2(0.05f), atlas(2, 0));
		//Renderer::RenderQuad(glm::vec3(-0.05, -0.75, 0.f), glm::vec2(0.05f), atlas(2, 0));

		angle += 2.f;
		if (angle > 360.f)
			angle -= 360.f;

		Renderer::End();
		window.SwapBuffers();
	}

	LOG(LOG_INFO, "Done.\n");
	return 0;
}