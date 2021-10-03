#include "breakout/game.h"

#include "breakout/window.h"

#include "breakout/renderer.h"
#include "breakout/resources.h"
#include "breakout/texture.h"
#include "breakout/utils.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

namespace Game {

	struct InputState {
		bool left = false;
		bool right = false;
	};

	struct InGameState {
		Platform p;
		Ball b;
		std::vector<Brick> bricks;

		GameState state;
		InputState inputs;
		float deltaTime = 0.f;

		int lives = 3;
		int level = 0;
		
		glm::ivec2 fieldSize;
		glm::vec2 brickSize;
		float fieldOffsetY = 0.2f;
	};

	struct GameResources {
		ShaderRef quadShader;
		AtlasTextureRef atlas;
		TextureRef background;
	};

	static GameResources res;
	static InGameState state;

	void DeltaTimeUpdate();
	void RenderScene();
	void ProcessInput();

	void Ingame_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	void Init() {
		Window& window = Window::Get();
		if (!window.IsInitialized()) {
			window.Init(1200, 900, "Breakout");
		}

		res.quadShader = Resources::TryGetShader("quads", "res/shaders/basic_quad_shader");
		res.atlas = std::make_shared<AtlasTexture>("res/textures/atlas01.png", glm::ivec2(128, 128));
		res.background = std::make_shared<Texture>("res/textures/background_ingame.png");
	}

	void Run() {
		//TODO: add proper menu, etc.
		Play();
	}

	bool LoadLevel(const char* filepath) {
		//load level description file
		std::string levelDesc;
		if (!TryReadFile(filepath, levelDesc)) {
			LOG(LOG_ERROR, "Level loading failed.\n");
			return false;
		}

		//previous level cleanup
		state.bricks.clear();
		state.fieldSize = glm::ivec2(0);

		//parse level data
		std::string::size_type prevPos = 0, pos = 0;
		while ((pos = levelDesc.find('\n', pos)) != std::string::npos) {
			std::string_view s = std::string_view(levelDesc.c_str() + prevPos, pos - prevPos);
			//printf("|-%s-|\n", levelDesc.substr(prevPos, pos - prevPos).c_str());
			prevPos = ++pos;

			//playing field size update
			if (state.fieldSize.x < s.size())
				state.fieldSize.x = s.size();
			if (s.size() > 1)
				state.fieldSize.y++;
			else
				continue;

			//parse bricks in this row
			for (int i = 0; i < s.size(); i++) {
				if (s[i] == ' ' || s[i] == '0')
					continue;

				int type = s[i] - '0';
				if (type < 0 || type > 9) {
					LOG(LOG_WARN, "Invalid brick types ('%c')\n", s[i]);
					continue;
					//TODO: maybe also allow characters
				}

				state.bricks.push_back(Brick(i, state.fieldSize.y - 1, type));
			}
		}
		//printf("|-%s-|\n", levelDesc.substr(prevPos, pos - prevPos).c_str());
		//printf("%d %d\n", state.fieldSize.x, state.fieldSize.y);

		//update brick sizes & positions
		state.brickSize = glm::vec2(
			1.f / state.fieldSize.x,
			(1.f - state.fieldOffsetY) / state.fieldSize.y
		);
		for (Brick& b : state.bricks) {
			b.pos = glm::vec2(
				-1.f + b.coords.x * state.brickSize.x * 2.f + state.brickSize.x,
				 1.f - b.coords.y * state.brickSize.y * 2.f - state.brickSize.y
			);
		}

		LOG(LOG_INFO, "Loaded level from '%s'.\n", filepath);
		return true;
	}

	void Play() {
		Window& window = Window::Get();

		Renderer::SetShader(res.quadShader);

		glfwSetKeyCallback(window.Handle(), Ingame_KeyCallback);

		state.state = GameState::Playing;
		DeltaTimeUpdate();

		LoadLevel("res/levels/level00.txt");

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		while (!window.ShouldClose()) {
			glClear(GL_COLOR_BUFFER_BIT);
			Renderer::Begin();

			DeltaTimeUpdate();
			
			switch (state.state) {
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
		Renderer::RenderQuad(glm::vec3(0.f, 0.f, 1.f), glm::vec2(1.f), res.background);

		//platform
		Renderer::RenderQuad(glm::vec3(state.p.pos, -0.96f, 0.f), glm::vec2(state.p.scale, 0.03f), glm::vec4(glm::vec3(0.3f), 1.f));
		Renderer::RenderQuad(glm::vec3(state.p.pos, -0.96f, 0.f), glm::vec2(state.p.scale, 0.03f) - 0.01f, glm::vec4(glm::vec3(0.7f), 1.f));

		//bricks
		for (Brick& b : state.bricks) {
			Renderer::RenderQuad(glm::vec3(b.pos, 0.f), glm::vec2(state.brickSize), glm::vec4(1.f, 0.f, 0.f, 1.f));
		}

		//ball
		glm::vec2 ballSize = glm::vec2(state.b.radius / Window::Get().AspectRatio(), state.b.radius);
		Renderer::RenderQuad(glm::vec3(state.b.pos, 0.f), ballSize, res.atlas->GetTexture(0, 0));
	}

	void ProcessInput() {
		if (state.state == GameState::Playing && (state.inputs.left || state.inputs.right)) {
			float move = (float(state.inputs.right) - float(state.inputs.left)) * state.p.speed;
			state.p.pos += move * state.deltaTime;

			//platform movement clamping
			if (state.p.pos < -0.98f + state.p.scale)
				state.p.pos = -0.98f + state.p.scale;
			if (state.p.pos > 0.98f - state.p.scale)
				state.p.pos = 0.98f - state.p.scale;
		}
	}

	void DeltaTimeUpdate() {
		static double prevTime = 0.0;

		double currTime = glfwGetTime();
		state.deltaTime = (currTime - prevTime);
		prevTime = currTime;
	}

	void Ingame_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		switch (key) {
			case GLFW_KEY_A:		//move platform left
				state.inputs.left = (action != GLFW_RELEASE);
				break;
			case GLFW_KEY_D:		//move platform right
				state.inputs.right = (action != GLFW_RELEASE);
				break;
			case GLFW_KEY_P:		//pause game
				if (action == GLFW_PRESS) {
					if (state.state == GameState::Playing)
						state.state = GameState::Paused;
					else if (state.state == GameState::Paused)
						state.state = GameState::Playing;
				}
				break;
			case GLFW_KEY_ESCAPE:	//ingame menu

				break;
		}
	}

}//namespace Game
