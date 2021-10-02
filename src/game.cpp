#include "breakout/game.h"

#include "breakout/window.h"
#include "breakout/renderer.h"
#include "breakout/resources.h"
#include "breakout/texture.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct InGameState {
	float platformPosition = 0.5f;
	float platformScale = 0.2f;
};

struct GameData {
	ShaderRef quadShader;
	AtlasTextureRef atlas;
	TextureRef background;

	InGameState state;
};

static GameData data;

namespace Game {

	void Init() {
		Window& window = Window::Get();
		if (!window.IsInitialized()) {
			window.Init(1200, 900, "Breakout");
		}

		data.quadShader = Resources::TryGetShader("quads", "res/shaders/basic_quad_shader");
		data.atlas = std::make_shared<AtlasTexture>("res/textures/test_atlas.png", glm::ivec2(128, 128));
		data.background = std::make_shared<Texture>("res/textures/background_ingame.png");
	}

	void Run() {
		Window& window = Window::Get();
		Renderer::SetShader(data.quadShader);

		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		while (!window.ShouldClose()) {
			glClear(GL_COLOR_BUFFER_BIT);
			Renderer::Begin();

			Renderer::RenderQuad(glm::vec3(0.f, 0.f, 1.f), glm::vec2(1.f), data.background);

			Renderer::RenderQuad(glm::vec3(0.f, 0.f, 0.f), glm::vec2(0.1f), glm::vec4(1.f, 0.f, 0.f, 1.f));
			Renderer::RenderQuad(glm::vec3(0.25f, 0.f, 0.f), glm::vec2(0.1f), glm::vec4(0.f, 1.f, 0.f, 1.f));
			Renderer::RenderQuad(glm::vec3(-0.25f, 0.f, 0.f), glm::vec2(0.1f), glm::vec4(0.f, 0.f, 1.f, 1.f));
			Renderer::RenderQuad(glm::vec3(0.0f, 0.25f, 0.f), glm::vec2(0.1f), glm::vec4(1.f, .5f, 0.2f, 1.f));
			Renderer::RenderQuad(glm::vec3(0.0f, -0.25f, 0.f), glm::vec2(0.1f), glm::vec4(1.f, 0.f, 1.f, 1.f));

			for (int x = 0; x < 5; x++) {
				for (int y = 0; y < 5; y++) {
					Renderer::RenderQuad(glm::vec3(-0.25 + 0.11 * x, -0.4 - 0.11 * y, 0.f), glm::vec2(0.05f), data.atlas->GetTexture(7 + x, 6 + y));
				}
			}


			Renderer::End();
			window.SwapBuffers();
		}

	}

}//namespace Game
