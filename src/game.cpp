#include "breakout/game.h"

#include "breakout/window.h"

#include "breakout/renderer.h"
#include "breakout/resources.h"
#include "breakout/texture.h"
#include "breakout/text.h"
#include "breakout/utils.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <filesystem>

namespace Game {

#define PLATFORM_Y_POS -0.96f
#define PLATFORM_HEIGHT 0.03f
#define PLATFORM_BOUNCE_STEEPNESS 1.5f

#define RESET_DELAY_SEC 1.25f

#define DEFAULT_BALL_SPEED 1.5f
#define DEFAULT_PLATFORM_SPEED 0.5f
#define DEFAULT_PLATFORM_SCALE 0.2f
#define STARTING_LIVES 3

	struct InputState {
		bool left = false;
		bool right = false;

		bool up = false;
		bool down = false;

		double mouseX = 0.0;
		double mouseY = 0.0;
	};

	typedef void (*TransitionHandlerType)();

	struct Button {
		typedef void(*ButtonCallbackType)();
	public:
		std::string name;
		Quad quad;
		ButtonCallbackType Callback = nullptr;
	public:
		Button() = default;
		Button(const std::string& name, const Quad& quad, ButtonCallbackType Callback);

		void Click();
		bool Hover();
	};
	\
	struct InGameState {
		Platform p;
		Ball b;
		std::vector<Brick> bricks;

		GameState state;
		InputState inputs;
		float deltaTime = 0.f;

		int lives = STARTING_LIVES;
		int level = 0;
		
		glm::ivec2 fieldSize;
		glm::vec2 brickSize;
		float fieldOffsetY = 0.2f;

		GameState transition_nextState;
		float transition_endTime = 0.f;
		float transition_startTime = 0.f;
		std::string transition_msg;
		bool transition_keepBallMoving = false;
		TransitionHandlerType TransitionHandler = nullptr;
		float transition_fadeIn = false;

		bool running = true;
		bool endScreen_gameWon = false;
		bool menu_optionsScreen = false;

		std::vector<Button> activeButtons;
	};

	struct GameResources {
		ShaderRef quadShader;
		AtlasTextureRef atlas;
		TextureRef background;
		FontRef font;

		std::vector<std::string> levelPaths;
	};

	static GameResources res;
	static InGameState state;

	static char textbuf[256];

	void DeltaTimeUpdate();
	void RenderScene();
	void GameUpdate();
	void CollisionResolution();
	void MidGame_Reset();
	void GameStateReset();

	void Transition_LoadLevel();
	void Transition_BallLost();

	void Ingame_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void Ingame_MouseCallback(GLFWwindow* window, int button, int action, int mods);

	void Btn_Resume();
	void Btn_Options();
	void Btn_OptionsBack();
	void Btn_Play();
	void Btn_MainMenu();
	void Btn_Reset();
	void Btn_Quit();

	bool LoadLevel(const char* filepath);

	bool MainMenu();
	void Play();

	//===== Collision handling =====

	//Collision detection between two AABBs.
	bool AABB_AABBCollision(const glm::vec2& aPos, const glm::vec2& aSize, const glm::vec2& bPos, const glm::vec2& bSize);
	//Ball-wall collision detection & resolution.
	//Returns true if ball touched the bottom wall -> player loses life.
	bool WallsCollision();
	bool Ball_PlatformCollision(glm::vec2& out_posFix, glm::vec2& out_newDir);
	bool Ball_BrickCollision(const Brick& brick, glm::vec2& out_posFix, glm::vec2& out_newDir);

	void RenderButton(const std::string& btnName, Button::ButtonCallbackType callback, const char* text, const glm::vec2& center, const glm::vec2& size, float fontScale, const ITextureRef& texture, const glm::vec4& fontColor = glm::vec4(1.f));
	void RenderButton2(const std::string& btnName, Button::ButtonCallbackType callback, const char* text, 
					   const glm::vec2& center, const glm::vec2& size, float fontScale, 
					   const ITextureRef& texture, const ITextureRef& texture2, const glm::vec4& fontColor = glm::vec4(1.f));

	//===== Game =====

	void MousePosUpdate() {
		double x, y;

		glfwGetCursorPos(Window::Get().Handle(), &x, &y);
		x /= Window::Get().Width();
		y /= Window::Get().Height();

		//invert vertical axis
		y = 1.f - y;

		//stretch to <-1,1> range (NDC coords)
		x = 2.f * x - 1.f;
		y = 2.f * y - 1.f;

		state.inputs.mouseX = x;
		state.inputs.mouseY = y;
	}

	//General initialization. Needs to be called before Run().
	void Init() {
		Window& window = Window::Get();
		if (!window.IsInitialized()) {
			window.Init(1200, 900, "Breakout");
		}

		res.quadShader = Resources::TryGetShader("quads", "res/shaders/basic_quad_shader");
		res.atlas = std::make_shared<AtlasTexture>("res/textures/atlas01.png", glm::ivec2(128, 128));
		res.background = std::make_shared<Texture>("res/textures/background_ingame.png");
		res.font = std::make_shared<Font>("res/fonts/PermanentMarker-Regular.ttf");

		//load all level filepaths
		try {
			LOG(LOG_INFO, "Levels:\n");
			for (auto& entry : std::filesystem::directory_iterator("res/levels/")) {
				if (entry.path().extension() == ".txt") {
					res.levelPaths.push_back(entry.path().string());
					LOG(LOG_INFO, "\t%s\n", res.levelPaths.back().c_str());
				}
			}
		}
		catch (std::exception&) {
			LOG(LOG_ERROR, "Levels directory not found.");
		}

		Renderer::SetShader(res.quadShader);

		//setup input callbacks
		glfwSetKeyCallback(window.Handle(), Ingame_KeyCallback);
		glfwSetMouseButtonCallback(window.Handle(), Ingame_MouseCallback);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void Run() {
		Game::Init();

		state.running = true;
		while (!Window::Get().ShouldClose() && state.running) {
			if (!MainMenu()) {
				//quit
				Window::Get().Close();
				state.running = false;
				break;
			}

			Play();
		}

	}

	bool MainMenu() {
		Window& window = Window::Get();

		state.menu_optionsScreen = false;

		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		while (!window.ShouldClose() && state.running && state.state == GameState::MainMenu) {
			glClear(GL_COLOR_BUFFER_BIT);
			Renderer::Begin();
			state.activeButtons.clear();

			MousePosUpdate();

			if (!state.menu_optionsScreen) {
				Renderer::RenderText_Centered(res.font, "BREAKOUT", glm::vec2(0.f, 0.7f), 4.f, glm::vec4(1.f));

				RenderButton2("btn_menu_play", Btn_Play, "Play", glm::vec2(0.f, 0.3f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
				RenderButton2("btn_menu_options", Btn_Options, "Options", glm::vec2(0.f, 0.1f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
				RenderButton2("btn_menu_quit", Btn_Quit, "Quit", glm::vec2(0.f, -0.1f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
			}
			else {
				Renderer::RenderText_Centered(res.font, "Options", glm::vec2(0.f, 0.55f), 3.f, glm::vec4(1.f));

				RenderButton2("btn_opt_back", Btn_OptionsBack, "Back", glm::vec2(0.f, -0.1f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
			}

			Renderer::End();
			window.SwapBuffers();
		}

		return true;
	}

	void Play() {
		Window& window = Window::Get();

		//state.state = GameState::Playing;
		DeltaTimeUpdate();

		state.level = 0;
		state.lives = STARTING_LIVES;
		if (!LoadLevel(res.levelPaths[state.level].c_str())) {
			throw std::exception();
		}

		GameStateReset();

		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		while (!window.ShouldClose() && state.running && state.state != GameState::MainMenu) {
			glClear(GL_COLOR_BUFFER_BIT);
			Renderer::Begin();
			state.activeButtons.clear();

			DeltaTimeUpdate();
			MousePosUpdate();
			
			switch (state.state) {
				case GameState::Playing:
					GameUpdate();
					RenderScene();
					break;
				case GameState::Paused:
					RenderScene();
					Renderer::RenderQuad(glm::vec3(0.f), glm::vec2(1.f), glm::vec4(glm::vec3(0.0f), 0.5f));
					Renderer::RenderText_Centered(res.font, "Game Paused", glm::vec2(0.f), 2.f, glm::vec4(1.f));
					break;
				case GameState::IngameMenu:
					RenderScene();
					Renderer::RenderQuad(glm::vec3(0.f), glm::vec2(1.f), glm::vec4(glm::vec3(0.0f), 0.5f));
					Renderer::RenderText_Centered(res.font, "BREAKOUT", glm::vec2(0.f, 0.7f), 4.f, glm::vec4(1.f));
					
					RenderButton2("btn_game_resume", Btn_Resume, "Resume", glm::vec2(0.f, 0.3f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					RenderButton2("btn_game_reset", Btn_Reset, "Reset game", glm::vec2(0.f, 0.1f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					RenderButton2("btn_game_menu", Btn_MainMenu, "Main menu", glm::vec2(0.f, -0.1f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					RenderButton2("btn_game_quit", Btn_Quit, "Quit", glm::vec2(0.f, -0.3f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					break;
				case GameState::Transition:
					RenderScene();
					if (state.transition_msg[0] != '\0') {
						Renderer::RenderText_Centered(res.font, state.transition_msg.c_str(), glm::vec2(0.f), 2.f, glm::vec4(1.f));
					}

					if (state.transition_keepBallMoving) {
						state.b.pos += state.b.dir * state.b.speed * state.deltaTime;
					}
					if (state.transition_fadeIn) {
						float alpha =  1.f - (glfwGetTime() - state.transition_startTime) / (state.transition_endTime - state.transition_startTime);
						Renderer::RenderQuad(glm::vec3(0.f), glm::vec2(1.f), glm::vec4(glm::vec3(0.0f), alpha));
					}

					if (state.transition_endTime < glfwGetTime()) {
						if (state.TransitionHandler != nullptr) {
							state.TransitionHandler();
						}
						else {
							state.state = state.transition_nextState;
						}
					}
					break;
				case GameState::EndScreen:
					if (state.endScreen_gameWon) {
						Renderer::RenderText_Centered(res.font, "You won!", glm::vec2(0.f, 0.3f), 2.f, glm::vec4(1.f));

						snprintf(textbuf, sizeof(textbuf), "Levels cleared: %d", state.level);
						Renderer::RenderText_Centered(res.font, textbuf, glm::vec2(-0.3f, 0.1f), 1.f, glm::vec4(1.f));
						snprintf(textbuf, sizeof(textbuf), "Lives remaining: %d", state.lives);
						Renderer::RenderText_Centered(res.font, textbuf, glm::vec2(0.3f, 0.1f), 1.f, glm::vec4(1.f));

						RenderButton2("btn_win_reset", Btn_Reset, "Play again", glm::vec2(0.f, -0.1f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
						RenderButton2("btn_win_menu", Btn_MainMenu, "Main menu", glm::vec2(0.f, -0.3f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
						RenderButton2("btn_win_quit", Btn_Quit, "Quit", glm::vec2(0.f, -0.5f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					}
					else {
						Renderer::RenderText_Centered(res.font, "You lost!", glm::vec2(0.f, 0.3f), 2.f, glm::vec4(1.f));

						snprintf(textbuf, sizeof(textbuf), "Levels cleared: %d", state.level);
						Renderer::RenderText_Centered(res.font, textbuf, glm::vec2(0.0f, 0.1f), 1.f, glm::vec4(1.f));

						RenderButton2("btn_lost_reset", Btn_Reset, "Play again", glm::vec2(0.f, -0.1f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
						RenderButton2("btn_lost_menu", Btn_MainMenu, "Main menu", glm::vec2(0.f, -0.3f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
						RenderButton2("btn_lost_quit", Btn_Quit, "Quit", glm::vec2(0.f, -0.5f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					}
					break;
			}

			Renderer::End();
			window.SwapBuffers();
		}
	}

	void Btn_Play() {
		state.state = GameState::Transition;

		state.transition_startTime = glfwGetTime();
		state.transition_endTime = state.transition_startTime + 0.5f;
		state.transition_keepBallMoving = false;
		state.transition_nextState = GameState::Playing;
		state.transition_fadeIn = true;
		state.transition_msg = "";
	}

	void Btn_Options() {
		state.menu_optionsScreen = true;
	}

	void Btn_OptionsBack() {
		state.menu_optionsScreen = false;
	}

	void Btn_Resume() {
		state.state = GameState::Playing;
	}

	void Btn_MainMenu() {
		state.state = GameState::MainMenu;
	}

	void Btn_Reset() {
		state.level = 0;
		state.lives = STARTING_LIVES;
		Transition_LoadLevel();
		GameStateReset();
		state.state = GameState::Playing;
	}

	void Btn_Quit() {
		Window::Get().Close();
		state.running = false;
	}

	void MidGame_Reset() {
		state.p.pos = 0.f;
		state.b.pos = glm::vec2(state.p.pos, PLATFORM_Y_POS + PLATFORM_HEIGHT + state.b.radius);
		state.b.dir = glm::vec2(0.0f, 1.f);
		state.b.onPlatform = true;
	}

	void GameStateReset() {
		state.b.speed = DEFAULT_BALL_SPEED;
		state.p.speed = DEFAULT_PLATFORM_SPEED;
		state.p.scale = DEFAULT_PLATFORM_SCALE;

		MidGame_Reset();
	}

	void RenderScene() {
		//background texture
		//Renderer::RenderQuad(glm::vec3(0.f, 0.f, 1.f), glm::vec2(1.f), res.background);

		//platform
		Renderer::RenderQuad(glm::vec3(state.p.pos, PLATFORM_Y_POS, 0.f), glm::vec2(state.p.scale, PLATFORM_HEIGHT), glm::vec4(glm::vec3(0.3f), 1.f));
		Renderer::RenderQuad(glm::vec3(state.p.pos, PLATFORM_Y_POS, 0.f), glm::vec2(state.p.scale, PLATFORM_HEIGHT) - 0.01f, glm::vec4(glm::vec3(0.7f), 1.f));

		//bricks
		for (Brick& b : state.bricks) {
			/*Renderer::RenderQuad(glm::vec3(b.pos, 0.f), glm::vec2(state.brickSize), glm::vec4(1.f, 0.f, 0.f, 1.f));*/
			Renderer::RenderQuad(glm::vec3(b.pos, 0.f), glm::vec2(state.brickSize), res.atlas->GetTexture(1,0));
		}

		//ball
		glm::vec2 ballSize = glm::vec2(state.b.radius / Window::Get().AspectRatio(), state.b.radius);
		Renderer::RenderQuad(glm::vec3(state.b.pos, 0.f), ballSize, res.atlas->GetTexture(0, 0));

		//texts
		snprintf(textbuf, sizeof(textbuf), "Lives: %d", state.lives);
		Renderer::RenderText(res.font, textbuf, glm::vec2(-0.95f, 0.9f), 1.f, glm::vec4(1.f));

		snprintf(textbuf, sizeof(textbuf), "Level: %d", state.level + 1);
		Renderer::RenderText(res.font, textbuf, glm::vec2(-0.95f, 0.8f), 1.f, glm::vec4(1.f));

		//Renderer::RenderQuad(glm::vec3(0.f, 0.f, 1.f), glm::vec2(1.f), res.font->GetAtlasTexture());
	}

	void GameUpdate() {
		float prevPos = state.p.pos;

		//input processing - platform movement
		if (state.state == GameState::Playing && (state.inputs.left || state.inputs.right)) {
			float move = (float(state.inputs.right) - float(state.inputs.left)) * state.p.speed;
			state.p.pos += move * state.deltaTime;

			//platform movement clamping
			if (state.p.pos < -0.98f + state.p.scale)
				state.p.pos = -0.98f + state.p.scale;
			if (state.p.pos > 0.98f - state.p.scale)
				state.p.pos = 0.98f - state.p.scale;
		}
		
		//ball update
		if (state.b.onPlatform) {
			//stick to the platform
			float ballXOffset = state.b.pos.x - prevPos;
			state.b.pos = glm::vec2(state.p.pos + ballXOffset, PLATFORM_Y_POS + PLATFORM_HEIGHT + state.b.radius);
		}
		else {
			//ball movement
			state.b.pos += state.b.dir * state.b.speed * state.deltaTime;

			//collision detection & resolution
			CollisionResolution();
		}

		//level finished condition check
		if (state.bricks.size() < 1) {
			state.level++;
			state.transition_keepBallMoving = true;
			state.transition_endTime = glfwGetTime() + RESET_DELAY_SEC;
			state.state = GameState::Transition;
			state.transition_msg = "Level finished!";
			state.TransitionHandler = Transition_LoadLevel;
			state.transition_fadeIn = false;
		}
	}

	void CollisionResolution() {
		glm::vec2 ballSize = glm::vec2(state.b.radius);
		std::vector<int> bricksDeleteIdx;
		glm::vec2 posFix, newDir;

		//collisions with bricks
		for(int i = 0; i < int(state.bricks.size()); i++) {
#if 0
			if(AABB_AABBCollision(state.b.pos, ballSize, state.bricks[i].pos, state.brickSize)) {
				bricksDeleteIdx.push_back(i);
				break;
			}
#else
			if (Ball_BrickCollision(state.bricks[i], posFix, newDir)) {
				bricksDeleteIdx.push_back(i);

				//fix ball's position and change direction (bounce)
				state.b.pos += posFix;
				state.b.dir = newDir;
				break;
			}
#endif
		}

		//delete marked bricks
		for (auto it = bricksDeleteIdx.rbegin(); it != bricksDeleteIdx.rend(); ++it) {
			state.bricks.erase(state.bricks.begin() + *it);
		}

		//collision with platform
		if (Ball_PlatformCollision(posFix, newDir)) {
			state.b.pos += posFix;
			state.b.dir = newDir;
			//state.b.onPlatform = true;
		}

		//collisions with walls (left/top/right -> bounce, bottom -> lose)
		if (WallsCollision()) {
			if ((--state.lives) <= 0) {
				state.transition_nextState = GameState::EndScreen;
				state.transition_endTime = glfwGetTime() + RESET_DELAY_SEC;
				state.transition_keepBallMoving = true;
				state.transition_msg = "Game over!";
				state.TransitionHandler = nullptr;
				state.transition_fadeIn = false;
				state.endScreen_gameWon = false;

				state.state = GameState::Transition;
				LOG(LOG_INFO, "Ball lost. Game over.\n");
			}
			else {
				state.transition_nextState = GameState::Playing;
				state.transition_endTime = glfwGetTime() + RESET_DELAY_SEC;
				state.transition_keepBallMoving = true;
				state.transition_msg = "Ball lost!";
				state.TransitionHandler = Transition_BallLost;
				state.transition_fadeIn = false;

				state.state = GameState::Transition;
				LOG(LOG_INFO, "Ball lost. Remaining lives: %d\n", state.lives);
			}
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
			case GLFW_KEY_W:		//menu controls - up
				state.inputs.up = (action != GLFW_RELEASE);
				break;
			case GLFW_KEY_S:		//menu controls - down
				state.inputs.down = (action != GLFW_RELEASE);
				break;
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
				if (action == GLFW_PRESS) {
					if (state.state == GameState::Playing)
						state.state = GameState::IngameMenu;
					else if (state.state == GameState::IngameMenu)
						state.state = GameState::Playing;
				}
				break;
			case GLFW_KEY_ENTER:	//menu controls - button click

				break;
			case GLFW_KEY_SPACE:	//fire the ball
				if (action == GLFW_PRESS && state.b.onPlatform) {
					state.b.onPlatform = false;
				}
				break;
		}
	}

	void Ingame_MouseCallback(GLFWwindow* window, int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
			MousePosUpdate();

			for (Button& btn : state.activeButtons) {
				if (btn.Hover()) {
					btn.Click();
					break;
				}
			}

		}
	}

	//===== Collision handling =====

	//Collision detection between two AABBs.
	bool AABB_AABBCollision(const glm::vec2& aPos, const glm::vec2& aSize, const glm::vec2& bPos, const glm::vec2& bSize) {
		glm::vec2 aMin = aPos - aSize;
		glm::vec2 aMax = aPos + aSize;
		glm::vec2 bMin = bPos - bSize;
		glm::vec2 bMax = bPos + bSize;

		bool xCheck = (aMin.x < bMax.x) && (aMax.x > bMin.x);
		bool yCheck = (aMin.y < bMax.y) && (aMax.y > bMin.y);

		return (xCheck && yCheck);
	}

	//Ball-wall collision detection & resolution.
	//Returns true if ball touched the bottom wall -> player loses life.
	bool WallsCollision() {
		glm::vec2 bMin = state.b.pos - state.b.radius;
		glm::vec2 bMax = state.b.pos + state.b.radius;

		if (bMin.x <= -1.f || bMax.x >= 1.f) {
			state.b.dir.x = -state.b.dir.x;
		}
		else if (bMax.y >= 1.f) {
			state.b.dir.y = -state.b.dir.y;
		}
		else if (bMin.y <= -1.f) {
			return true;
		}

		return false;
	}

	bool Ball_PlatformCollision(glm::vec2& out_posFix, glm::vec2& out_newDir) {
		glm::vec2 pMin = glm::vec2(state.p.pos - state.p.scale, PLATFORM_Y_POS - PLATFORM_HEIGHT);
		glm::vec2 pMax = glm::vec2(state.p.pos + state.p.scale, PLATFORM_Y_POS + PLATFORM_HEIGHT);

		glm::vec2 P = glm::clamp(state.b.pos, pMin, pMax);
		float dist = glm::distance(P, state.b.pos);
		if (dist < state.b.radius) {
			//compute position fix vector
			glm::vec2 a = (P - state.b.pos);
			glm::vec2 b = a * (state.b.radius / dist);
			out_posFix = a - b;

			////new direction -> based on distance from platform's center
			//out_newDir = state.b.dir;
			//if (P.x == pMin.x || P.x == pMax.x) {
			//	//collision with left/right side of brick
			//	out_newDir.x = -out_newDir.x;
			//}
			//else {
			//	//collision with top/bottom side of brick
			//	out_newDir.y = -out_newDir.y;
			//}
			////float f = 0.5f + (fabsf(P.x - state.p.pos) / state.p.scale);
			////out_newDir.x *= f;
			////out_newDir = glm::normalize(out_newDir);

			float f = ((P.x - state.p.pos) / state.p.scale) * PLATFORM_BOUNCE_STEEPNESS;
			out_newDir = glm::normalize(glm::vec2(f, 1.f));

			printf("%.2f\n", f);

			return true;
		}

		return false;
	}

	bool Ball_BrickCollision(const Brick& brick, glm::vec2& out_posFix, glm::vec2& out_newDir) {
		glm::vec2 brickMin = brick.pos - state.brickSize;
		glm::vec2 brickMax = brick.pos + state.brickSize;

		glm::vec2 P = glm::clamp(state.b.pos, brickMin, brickMax);

		float dist = glm::distance(P, state.b.pos);
		if (dist < state.b.radius) {
			//compute position fix vector
			glm::vec2 a = (P - state.b.pos);
			glm::vec2 b = a * (state.b.radius / dist);
			out_posFix = a - b;

			//glm::vec2 N = ...;
			//out_newDir = glm::normalize(state.b.dir - 2.f * glm::dot(state.b.dir, N) * N);

			//compute bounce direction
			out_newDir = state.b.dir;
			if (P.x == brickMin.x || P.x == brickMax.x) {
				//collision with left/right side of brick
				out_newDir.x = -out_newDir.x;
			}
			else {
				//collision with top/bottom side of brick
				out_newDir.y = -out_newDir.y;
			}

			return true;
		}
		else
			return false;
	}

	void RenderButton(const std::string& btnName, Button::ButtonCallbackType callback, const char* text, const glm::vec2& center, const glm::vec2& size, float fontScale, const ITextureRef& texture, const glm::vec4& fontColor) {
		Renderer::RenderQuad(glm::vec3(center, 0.f), size, texture);
		state.activeButtons.push_back(Button(btnName, Renderer::GetLastQuad(), callback));

		Renderer::RenderText_Centered(res.font, text, center, fontScale, fontColor);
	}

	void RenderButton2(const std::string& btnName, Button::ButtonCallbackType callback, const char* text, const glm::vec2& center, const glm::vec2& size, float fontScale, const ITextureRef& texture, const ITextureRef& texture2, const glm::vec4& fontColor) {
		Renderer::RenderQuad(glm::vec3(center, 0.f), size, texture);
		state.activeButtons.push_back(Button(btnName, Renderer::GetLastQuad(), callback));
		if (state.activeButtons.back().Hover()) {
			Renderer::RenderQuad(glm::vec3(center, 0.f), size, texture2);
		}

		Renderer::RenderText_Centered(res.font, text, center, fontScale, fontColor);
	}

	void Transition_BallLost() {
		MidGame_Reset();
		state.state = GameState::Playing;
	}

	void Transition_LoadLevel() {
		if (res.levelPaths.size() <= state.level) {
			//no more levels -> game finished
			state.endScreen_gameWon = true;
			state.state = GameState::EndScreen;
		}
		else {
			//load next level & reset state
			if (!LoadLevel(res.levelPaths[state.level].c_str())) {
				LOG(LOG_WARN, "Level loading error ('%s').\n", res.levelPaths[state.level].c_str());
				throw std::exception();
			}
			GameStateReset();
			state.state = GameState::Playing;
		}
	}

	Button::Button(const std::string& name_, const Quad& quad_, ButtonCallbackType Callback_) : name(name_), quad(quad_), Callback(Callback_) {}

	void Button::Click() {
		LOG(LOG_INFO, "Button [%s] clicked.\n", name.c_str());
		if (Callback != nullptr) {
			Callback();
		}
	}

	bool Button::Hover() {
		double x = state.inputs.mouseX;
		double y = state.inputs.mouseY;

		glm::vec2 bMin = glm::vec2(quad.vertices[0].position);
		glm::vec2 bMax = glm::vec2(quad.vertices[3].position);

		return (x >= bMin.x && x <= bMax.x && y >= bMin.y && y <= bMax.y);
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

}//namespace Game
