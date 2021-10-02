#include "breakout/game.h"

#include "breakout/window.h"

#include "breakout/renderer.h"
#include "breakout/resources.h"
#include "breakout/texture.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

enum class GameState {
	MainMenu,
	Playing,
	Paused,
	IngameMenu,
};

struct Platform {
	float pos = 0.f;
	float scale = 0.2f;
	float speed = 0.5f;
};

struct InGameState {
	Platform p;

	float deltaTime = 0.f;

	GameState state;
};

struct InputState {
	bool left = false;
	bool right = false;
};

struct GameData {
	ShaderRef quadShader;
	AtlasTextureRef atlas;
	TextureRef background;

	InGameState state;
	InputState inputs;
};

static GameData data;

namespace Game {

	void DeltaTimeUpdate();
	void RenderScene();
	void ProcessInput();

	void Init() {
		Window& window = Window::Get();
		if (!window.IsInitialized()) {
			window.Init(1200, 900, "Breakout");
		}

		data.quadShader = Resources::TryGetShader("quads", "res/shaders/basic_quad_shader");
		data.atlas = std::make_shared<AtlasTexture>("res/textures/atlas01.png", glm::ivec2(128, 128));
		data.background = std::make_shared<Texture>("res/textures/background_ingame.png");
	}

	void Run() {
		//TODO: add proper menu, etc.
		Play();
	}

	void Ingame_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		switch (key) {
			case GLFW_KEY_A:		//move platform left
				data.inputs.left = (action != GLFW_RELEASE);
				break;
			case GLFW_KEY_D:		//move platform right
				data.inputs.right = (action != GLFW_RELEASE);
				break;
			case GLFW_KEY_P:		//pause game
				if(action == GLFW_PRESS) {
					if (data.state.state == GameState::Playing)
						data.state.state = GameState::Paused;
					else if (data.state.state == GameState::Paused)
						data.state.state = GameState::Playing;
				}
				break;
			case GLFW_KEY_ESCAPE:	//ingame menu

				break;
		}
	}

	void Play() {
		Window& window = Window::Get();

		Renderer::SetShader(data.quadShader);

		glfwSetKeyCallback(window.Handle(), Ingame_KeyCallback);

		data.state.state = GameState::Playing;
		DeltaTimeUpdate();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		while (!window.ShouldClose()) {
			glClear(GL_COLOR_BUFFER_BIT);
			Renderer::Begin();

			DeltaTimeUpdate();
			
			switch (data.state.state) {
				case GameState::Playing:
					ProcessInput();
					RenderScene();
					break;
				case GameState::Paused:
					RenderScene();
					Renderer::RenderQuad(glm::vec3(0.f), glm::vec2(1.f), glm::vec4(glm::vec3(0.0f), 0.5f));
					break;
				case GameState::IngameMenu:
					RenderScene();
					Renderer::RenderQuad(glm::vec3(0.f), glm::vec2(1.f), glm::vec4(glm::vec3(0.0f), 0.5f));
					//TODO: render menu stuff
					break;
			}

			Renderer::End();
			window.SwapBuffers();
		}
	}

	void RenderScene() {
		//background texture
		Renderer::RenderQuad(glm::vec3(0.f, 0.f, 1.f), glm::vec2(1.f), data.background);

		//platform
		Renderer::RenderQuad(glm::vec3(data.state.p.pos, -0.96f, 0.f), glm::vec2(data.state.p.scale, 0.03f), glm::vec4(glm::vec3(0.3f), 1.f));
		Renderer::RenderQuad(glm::vec3(data.state.p.pos, -0.96f, 0.f), glm::vec2(data.state.p.scale, 0.03f) - 0.01f, glm::vec4(glm::vec3(0.7f), 1.f));
	}

	void ProcessInput() {
		if (data.state.state == GameState::Playing && (data.inputs.left || data.inputs.right)) {
			float move = (float(data.inputs.right) - float(data.inputs.left)) * data.state.p.speed;
			data.state.p.pos += move * data.state.deltaTime;

			//platform movement clamping
			if (data.state.p.pos < -0.98f + data.state.p.scale)
				data.state.p.pos = -0.98f + data.state.p.scale;
			if (data.state.p.pos > 0.98f - data.state.p.scale)
				data.state.p.pos = 0.98f - data.state.p.scale;
		}
	}

	void DeltaTimeUpdate() {
		static double prevTime = 0.0;

		double currTime = glfwGetTime();
		data.state.deltaTime = (currTime - prevTime);
		prevTime = currTime;
	}

}//namespace Game
