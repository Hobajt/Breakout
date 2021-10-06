#include "breakout/game.h"

#include "breakout/window.h"

#include "breakout/renderer.h"
#include "breakout/resources.h"
#include "breakout/texture.h"
#include "breakout/text.h"
#include "breakout/utils.h"
#include "breakout/framebuffer.h"
#include "breakout/particles.h"
#include "breakout/sound.h"

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
#define FADEIN_DURATION_SEC 0.5f

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

	enum class MenuState {
		Menu, Options, Play
	};

	enum class PostProcEffectType { None, Blur, Drunk, Chaos, Confuse };
	
	struct Effects {
		bool wallBreaker = false;
		bool platformSticking = false;

		PostProcEffectType postprocEffect = PostProcEffectType::None;
	};

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
		MenuState menuState = MenuState::Menu;

		std::vector<Button> activeButtons;
		Effects effects;
		int bricksLeft = 0;

		ParticleSystem emission;
	};

	struct GameResources {
		ShaderRef quadShader;
		ShaderRef postprocShader;

		AtlasTextureRef atlas;
		TextureRef background;
		FontRef fontBig;
		FontRef fontSmall;

		FramebufferRef fbo;

		std::vector<std::string> levelPaths;

		std::map<std::string, Sound::AudioRef> sounds;
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
	void TransitionLogic();

	void Ingame_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void Ingame_MouseCallback(GLFWwindow* window, int button, int action, int mods);

	void BallEmission_ParticleUpdate(std::vector<Particle>& particles_prev, std::vector<Particle>& particles, float deltaTime);

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

	void OnResizeCallback(int width, int height) {
		res.fbo->Resize(width, height);
	}

	//General initialization. Needs to be called before Run().
	void Init() {
		Window& window = Window::Get();
		if (!window.IsInitialized()) {
			window.Init(1200, 900, "Breakout");
		}

		res.quadShader = Resources::TryGetShader("quads", "res/shaders/basic_quad_shader");
		res.postprocShader = Resources::TryGetShader("postproc", "res/shaders/postproc_shader");
		res.atlas = std::make_shared<AtlasTexture>("res/textures/atlas01.png", glm::ivec2(128, 128));
		res.background = std::make_shared<Texture>("res/textures/background_ingame.png");
		res.fontSmall = std::make_shared<Font>("res/fonts/PermanentMarker-Regular.ttf", 48);
		res.fontBig = std::make_shared<Font>("res/fonts/PermanentMarker-Regular.ttf", 96);

		res.sounds["powerup"] = std::make_shared < Sound::Audio>("res/sounds/powerup.wav");
		res.sounds["bleep"] = std::make_shared < Sound::Audio>("res/sounds/bleep.mp3");
		res.sounds["beep"] = std::make_shared < Sound::Audio>("res/sounds/bleep.wav");
		res.sounds["solid"] = std::make_shared < Sound::Audio>("res/sounds/solid.wav");
		res.sounds["bang"] = std::make_shared < Sound::Audio>("res/sounds/thud-bang.mp3");
		res.sounds["lose"] = std::make_shared < Sound::Audio>("res/sounds/lose-retro.mp3");
		res.sounds["scratch"] = std::make_shared<Sound::Audio>("res/sounds/scratch3.mp3");

		state.emission = ParticleSystem(BallEmission_ParticleUpdate, 100);

		srand((unsigned int)glfwGetTime());

		TextureParams tParams = {};
		tParams.wrapping = GL_REPEAT;
		res.fbo = std::make_shared<Framebuffer>(window.Width(), window.Height(), GL_RGBA, tParams);
		window.SetResizeCallback(OnResizeCallback);

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

	void Release() {
		res.background = nullptr;
		res.quadShader = nullptr;
		res.postprocShader = nullptr;
		res.atlas = nullptr;

		res.fontBig = nullptr;
		res.fontSmall = nullptr;
		res.fbo = nullptr;

		res.levelPaths.clear();
		res.sounds.clear();

		Sound::Release();
		Resources::Clear();
		Renderer::Release();
	}

	bool MainMenu() {
		Window& window = Window::Get();
		Sound::Play(res.sounds["powerup"]);

		state.menuState = MenuState::Menu;

		glClearColor(0.1f, 0.1f, 0.1f, 1.f);
		while (!window.ShouldClose() && state.running && (state.state == GameState::MainMenu || state.state == GameState::Transition) && state.menuState != MenuState::Play) {
			glClear(GL_COLOR_BUFFER_BIT);
			Renderer::Begin();
			state.activeButtons.clear();

			switch (state.menuState) {
				case MenuState::Menu:
					Renderer::RenderText_Centered(res.fontBig, "BREAKOUT", glm::vec2(0.f, 0.7f), 2.f, glm::vec4(1.f));

					RenderButton2("btn_menu_play", Btn_Play, "Play", glm::vec2(0.f, 0.3f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					RenderButton2("btn_menu_options", Btn_Options, "Options", glm::vec2(0.f, 0.1f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					RenderButton2("btn_menu_quit", Btn_Quit, "Quit", glm::vec2(0.f, -0.1f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					break;
				case MenuState::Options:
					Renderer::RenderText_Centered(res.fontBig, "Options", glm::vec2(0.f, 0.55f), 1.5f, glm::vec4(1.f));
					RenderButton2("btn_opt_back", Btn_OptionsBack, "Back", glm::vec2(0.f, -0.1f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					break;
			}

			switch (state.state) {
				case GameState::MainMenu:
					MousePosUpdate();
					break;
				case GameState::Transition:
					TransitionLogic();
					break;
			}

			Renderer::End();
			window.SwapBuffers();
		}

		return true;
	}

	void TransitionLogic() {
		if (state.transition_msg[0] != '\0') {
			Renderer::RenderText_Centered(res.fontSmall, state.transition_msg.c_str(), glm::vec2(0.f), 2.f, glm::vec4(1.f));
		}

		if (state.transition_keepBallMoving) {
			state.b.pos += state.b.dir * state.b.speed * state.deltaTime;
		}
		if (state.transition_fadeIn) {
			float alpha = 1.f - (glfwGetTime() - state.transition_startTime) / (state.transition_endTime - state.transition_startTime);
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
	}

	void Play() {
		Sound::Play(res.sounds["powerup"]);
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
		while (!window.ShouldClose() && state.running && state.state != GameState::MainMenu && state.menuState != MenuState::Menu) {
			res.fbo->Bind();
			glClear(GL_COLOR_BUFFER_BIT);

			Renderer::UseFBO(res.fbo);
			Renderer::SetShader(res.quadShader);
			Renderer::Begin();

			state.activeButtons.clear();

			DeltaTimeUpdate();
			MousePosUpdate();
			
			switch (state.state) {
				case GameState::Playing:
					GameUpdate();
					RenderScene();
					break;
				case GameState::Transition:
					RenderScene();
					TransitionLogic();
					break;
				case GameState::Paused:
				case GameState::IngameMenu:
					RenderScene();
					break;
				case GameState::EndScreen:
					break;
			}
			Renderer::End();

			Framebuffer::Unbind();
			glClear(GL_COLOR_BUFFER_BIT);
			Renderer::UseFBO(nullptr);

			//==== postprocessing render pass ====
			Renderer::SetShader(res.postprocShader);
			Renderer::Begin();
			Renderer::RenderQuad(glm::vec3(0.f), glm::vec2(1.f), res.fbo->GetTexture());
			Renderer::End();

			//==== GUI render pass (done separately, so that post-processing isn't applied) ====
			Renderer::SetShader(res.quadShader);
			Renderer::Begin();
			switch (state.state) {
				case GameState::Paused:
					RenderScene();
					Renderer::RenderQuad(glm::vec3(0.f), glm::vec2(1.f), glm::vec4(glm::vec3(0.0f), 0.5f));
					Renderer::RenderText_Centered(res.fontSmall, "Game Paused", glm::vec2(0.f), 2.f, glm::vec4(1.f));
					break;
				case GameState::IngameMenu:
					Renderer::RenderQuad(glm::vec3(0.f), glm::vec2(1.f), glm::vec4(glm::vec3(0.0f), 0.5f));
					Renderer::RenderText_Centered(res.fontBig, "BREAKOUT", glm::vec2(0.f, 0.7f), 2.f, glm::vec4(1.f));

					RenderButton2("btn_game_resume", Btn_Resume, "Resume", glm::vec2(0.f, 0.3f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					RenderButton2("btn_game_reset", Btn_Reset, "Reset game", glm::vec2(0.f, 0.1f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					RenderButton2("btn_game_menu", Btn_MainMenu, "Main menu", glm::vec2(0.f, -0.1f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					RenderButton2("btn_game_quit", Btn_Quit, "Quit", glm::vec2(0.f, -0.3f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					break;
				case GameState::EndScreen:
					state.effects.postprocEffect = PostProcEffectType::None;
					res.postprocShader->Bind();
					res.postprocShader->SetInt("effect", 0);
					if (state.endScreen_gameWon) {
						Renderer::RenderText_Centered(res.fontSmall, "You won!", glm::vec2(0.f, 0.3f), 2.f, glm::vec4(1.f));

						snprintf(textbuf, sizeof(textbuf), "Levels cleared: %d", state.level);
						Renderer::RenderText_Centered(res.fontSmall, textbuf, glm::vec2(-0.3f, 0.1f), 1.f, glm::vec4(1.f));
						snprintf(textbuf, sizeof(textbuf), "Lives remaining: %d", state.lives);
						Renderer::RenderText_Centered(res.fontSmall, textbuf, glm::vec2(0.3f, 0.1f), 1.f, glm::vec4(1.f));

						RenderButton2("btn_win_reset", Btn_Reset, "Play again", glm::vec2(0.f, -0.1f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
						RenderButton2("btn_win_menu", Btn_MainMenu, "Main menu", glm::vec2(0.f, -0.3f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
						RenderButton2("btn_win_quit", Btn_Quit, "Quit", glm::vec2(0.f, -0.5f), glm::vec2(0.2f, 0.07f), 1.f, res.atlas->GetTexture(0, 1), res.atlas->GetTexture(1, 1));
					}
					else {
						Renderer::RenderText_Centered(res.fontSmall, "You lost!", glm::vec2(0.f, 0.3f), 2.f, glm::vec4(1.f));

						snprintf(textbuf, sizeof(textbuf), "Levels cleared: %d", state.level);
						Renderer::RenderText_Centered(res.fontSmall, textbuf, glm::vec2(0.0f, 0.1f), 1.f, glm::vec4(1.f));

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
		state.menuState = MenuState::Play;

		state.transition_startTime = glfwGetTime();
		state.transition_endTime = state.transition_startTime + FADEIN_DURATION_SEC;
		state.transition_keepBallMoving = false;
		state.transition_nextState = GameState::Playing;
		state.transition_fadeIn = true;
		state.transition_msg = "";
	}

	void Btn_Options() {
		state.menuState = MenuState::Options;
	}

	void Btn_OptionsBack() {
		state.menuState = MenuState::Menu;
	}

	void Btn_Resume() {
		state.state = GameState::Playing;
	}

	void Btn_MainMenu() {
		state.state = GameState::Transition;
		state.menuState = MenuState::Menu;

		state.transition_startTime = glfwGetTime();
		state.transition_endTime = state.transition_startTime + FADEIN_DURATION_SEC;
		state.transition_keepBallMoving = false;
		state.transition_nextState = GameState::MainMenu;
		state.TransitionHandler = nullptr;
		state.transition_fadeIn = true;
		state.transition_msg = "";
	}

	void Btn_Reset() {
		state.level = 0;
		state.lives = STARTING_LIVES;
		Transition_LoadLevel();
		GameStateReset();
		//state.state = GameState::Playing;

		state.state = GameState::Transition;
		state.transition_startTime = glfwGetTime();
		state.transition_endTime = state.transition_startTime + FADEIN_DURATION_SEC;
		state.transition_keepBallMoving = false;
		state.transition_nextState = GameState::Playing;
		state.transition_fadeIn = true;
		state.transition_msg = "";
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

		state.effects.wallBreaker = false;
		state.effects.platformSticking = false;
		if (state.effects.postprocEffect != PostProcEffectType::None) {
			res.postprocShader->Bind();
			res.postprocShader->SetInt("effect", 0);
			state.effects.postprocEffect = PostProcEffectType::None;
		}

		state.emission.Reset();
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

		//ball emission
		state.emission.Render();

		//platform
		Renderer::RenderQuad(glm::vec3(state.p.pos, PLATFORM_Y_POS, 0.f), glm::vec2(state.p.scale, PLATFORM_HEIGHT), glm::vec4(glm::vec3(0.3f), 1.f));
		Renderer::RenderQuad(glm::vec3(state.p.pos, PLATFORM_Y_POS, 0.f), glm::vec2(state.p.scale, PLATFORM_HEIGHT) - 0.01f, glm::vec4(glm::vec3(0.7f), 1.f));

		//bricks
		for (Brick& b : state.bricks) {
			/*Renderer::RenderQuad(glm::vec3(b.pos, 0.f), glm::vec2(state.brickSize), glm::vec4(1.f, 0.f, 0.f, 1.f));*/
			Renderer::RenderQuad(glm::vec3(b.pos, 0.f), glm::vec2(state.brickSize), res.atlas->GetTexture(b.color, 0));
			if (b.type != BrickType::Brick) {
				Renderer::RenderQuad(glm::vec3(b.pos, 0.f), glm::vec2(state.brickSize), res.atlas->GetTexture(b.tc.x, b.tc.y));
			}
		}

		//ball
		glm::vec2 ballSize = glm::vec2(state.b.radius / Window::Get().AspectRatio(), state.b.radius);
		Renderer::RenderQuad(glm::vec3(state.b.pos, 0.f), ballSize, res.atlas->GetTexture(0, 0));

		//texts
		snprintf(textbuf, sizeof(textbuf), "Lives: %d", state.lives);
		Renderer::RenderText(res.fontSmall, textbuf, glm::vec2(-0.95f, 0.9f), 1.f, glm::vec4(1.f));

		snprintf(textbuf, sizeof(textbuf), "Level: %d", state.level + 1);
		Renderer::RenderText(res.fontSmall, textbuf, glm::vec2(-0.95f, 0.8f), 1.f, glm::vec4(1.f));

		

		//Renderer::RenderQuad(glm::vec3(0.f, 0.f, 1.f), glm::vec2(1.f), res.font->GetAtlasTexture());
	}

	void GameUpdate() {
		float prevPos = state.p.pos;

		//ball emission update
		state.emission.Update(state.deltaTime);

		if (state.effects.postprocEffect == PostProcEffectType::Blur) {
			res.postprocShader->Bind();
			res.postprocShader->SetFloat("offset", 3.f / float(Window::Get().Height()));
			res.postprocShader->SetVec2("shakeVec", glm::normalize(glm::vec2(float(rand() * 2.f - 1.f) / RAND_MAX, float(rand() * 2.f - 1.f) / RAND_MAX)) * (0.1f * float(rand()) / RAND_MAX));
		}
		else if (state.effects.postprocEffect == PostProcEffectType::Drunk) {
			res.postprocShader->Bind();
			res.postprocShader->SetFloat("offset", 3.f * (1.f + (float(rand()) / RAND_MAX) * 2.f) / float(Window::Get().Height()));
			res.postprocShader->SetVec2("shakeVec", glm::vec2(0.f));
		}

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

		state.bricksLeft = 0;
		for (Brick& b : state.bricks) {
			if (b.type == BrickType::Brick)
				state.bricksLeft++;
		}

		//level finished condition check
		if (state.bricksLeft < 1) {
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
				bool bounce = false;

				switch (state.bricks[i].type) {
					default:
					case BrickType::Brick:
						Sound::Play(res.sounds["solid"]);
						bricksDeleteIdx.push_back(i);
						bounce = true;
						break;
					case BrickType::Wall:
						Sound::Play(res.sounds["solid"]);
						if (state.effects.wallBreaker) {
							bricksDeleteIdx.push_back(i);
						}
						bounce = true;
						break;
					case BrickType::PlatformGrow:
						Sound::Play(res.sounds["powerup"]);
						state.p.scale *= 2.f;
						bricksDeleteIdx.push_back(i);
						bounce = true;
						break;
					case BrickType::PlatformShrink:
						Sound::Play(res.sounds["powerup"]);
						state.p.scale *= 0.5f;
						bricksDeleteIdx.push_back(i);
						bounce = true;
						break;
					case BrickType::PlatformSticking:
						Sound::Play(res.sounds["powerup"]);
						state.effects.platformSticking = true;
						bricksDeleteIdx.push_back(i);
						bounce = true;
						break;
					case BrickType::WallBreaker:
						Sound::Play(res.sounds["powerup"]);
						state.effects.wallBreaker = true;
						bricksDeleteIdx.push_back(i);
						bounce = true;
						break;
					case BrickType::BallSpeedUp:
						Sound::Play(res.sounds["powerup"]);
						state.b.speed *= 1.5f;
						bricksDeleteIdx.push_back(i);
						bounce = true;
						break;
					case BrickType::BallSlowDown:
						Sound::Play(res.sounds["powerup"]);
						state.b.speed *= 0.666666f;
						bricksDeleteIdx.push_back(i);
						bounce = true;
						break;
					case BrickType::EffectBlur:
						Sound::Play(res.sounds["bleep"]);
						state.effects.postprocEffect = PostProcEffectType::Blur;
						res.postprocShader->Bind();
						res.postprocShader->SetInt("effect", 1);
						bricksDeleteIdx.push_back(i);
						bounce = true;
						break;
					case BrickType::EffectDrunk:
						Sound::Play(res.sounds["bleep"]);
						state.effects.postprocEffect = PostProcEffectType::Drunk;
						res.postprocShader->Bind();
						res.postprocShader->SetInt("effect", 2);
						bricksDeleteIdx.push_back(i);
						bounce = true;
						break;
					case BrickType::EffectChaos:
						Sound::Play(res.sounds["bleep"]);
						state.effects.postprocEffect = PostProcEffectType::Chaos;
						res.postprocShader->Bind();
						res.postprocShader->SetInt("effect", 3);
						bricksDeleteIdx.push_back(i);
						bounce = true;
						break;
					case BrickType::EffectConfuse:
						Sound::Play(res.sounds["bleep"]);
						state.effects.postprocEffect = PostProcEffectType::Confuse;
						res.postprocShader->Bind();
						res.postprocShader->SetInt("effect", 4);
						bricksDeleteIdx.push_back(i);
						bounce = true;
						break;
				}

				if (bounce) {
					//fix ball's position and change direction (bounce)
					state.b.pos += posFix;
					state.b.dir = newDir;
				}

				//if (isnan(state.b.pos.x) || isnan(state.b.pos.y)) { __debugbreak(); }

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
			if (state.effects.platformSticking) {
				state.b.onPlatform = true;
			}
			Sound::Play(res.sounds["bang"]);
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
				Sound::Play(res.sounds["lose"]);
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
				Sound::Play(res.sounds["scratch"]);
				LOG(LOG_INFO, "Ball lost. Remaining lives: %d\n", state.lives);
			}
		}

	}

	void DeltaTimeUpdate() {
		static double prevTime = 0.0;

		double currTime = glfwGetTime();
		state.deltaTime = (currTime - prevTime);
		prevTime = currTime;

		if (state.deltaTime > 1.f) {
			state.deltaTime = 0.1f;
		}
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

			//printf("%.2f\n", f);

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
			if (dist > 1e-3f) {
				glm::vec2 b = a * (state.b.radius / dist);
				out_posFix = a - b;
			}
			else {
				out_posFix = a;
			}

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

		Renderer::RenderText_Centered(res.fontSmall, text, center, fontScale, fontColor);
	}

	void RenderButton2(const std::string& btnName, Button::ButtonCallbackType callback, const char* text, const glm::vec2& center, const glm::vec2& size, float fontScale, const ITextureRef& texture, const ITextureRef& texture2, const glm::vec4& fontColor) {
		Renderer::RenderQuad(glm::vec3(center, 0.f), size, texture);
		state.activeButtons.push_back(Button(btnName, Renderer::GetLastQuad(), callback));
		if (state.activeButtons.back().Hover()) {
			Renderer::RenderQuad(glm::vec3(center, 0.f), size, texture2);
		}

		Renderer::RenderText_Centered(res.fontSmall, text, center, fontScale, fontColor);
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
			int rowLen = s.size() / 2;
			if (state.fieldSize.x < rowLen)
				state.fieldSize.x = rowLen;
			if (s.size() > 1)
				state.fieldSize.y++;
			else
				continue;

			//parse bricks in this row
			for (int i = 0; i < s.size() / 2; i++) {
				char color = s[i * 2];
				char type = s[i * 2 + 1];

				int c = 1;
				int t = BrickType::Brick;

				//empty space
				if (type == ' ' || type == '0')
					continue;

				//read block color (or keep default if invalid)
				if (color >= '1' && color <= '9') {
					c = int(color - '0');
				}

				//block type resolution
				const char* code = strchr(BrickType::BrickCodes(), type);
				if (code != nullptr) {
					t = int(code - BrickType::BrickCodes());
					state.bricks.push_back(Brick(i, state.fieldSize.y - 1, t, c));
				}
				else {
					LOG(LOG_WARN, "Invalid brick type ('%c')\n", s[i]);
				}
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

	namespace BrickType {
		const char* brickCodes = "BWGSTKUDLRCF";

		const char* BrickCodes() {
			return brickCodes;
		}

		glm::ivec2 GetTypeTexCoord(int type, int color) {
			static glm::ivec2 tc[] = {
				glm::ivec2(1,0),		//brick - shouldn't ever be picked
				glm::ivec2(0,2),		//wall
				glm::ivec2(1,2),		//platform grow
				glm::ivec2(2,2),		//platform shrink
				glm::ivec2(3,2),		//platform sticking
				glm::ivec2(4,2),		//wall breaker
				glm::ivec2(5,2),		//ball speed up
				glm::ivec2(6,2),		//ball slow down

				glm::ivec2(0,3),		//blur
				glm::ivec2(1,3),		//drunk
				glm::ivec2(2,3),		//chaos
				glm::ivec2(3,3),		//confuse
			};

			if (type == BrickType::Brick)
				return glm::ivec2(color, 0);
			else
				return tc[type];
		}
	}

	static glm::vec4 color_lerp(const glm::vec4& a, const glm::vec4& b, float t) {
		return b * t + a * (1.f - t);
	}

	void BallEmission_ParticleUpdate(std::vector<Particle>& particles_prev, std::vector<Particle>& particles, float deltaTime) {
		//process particles from previous frame
		particles.clear();
		for (Particle& p : particles_prev) {
			p.lifespan -= deltaTime;

			if (p.lifespan > 0.f) {
				p.position += p.velocity * deltaTime;
				p.angle_rad += p.angularVelocity * deltaTime;
				p.scale += p.deltaScale * deltaTime;
				
				//prematurely discard way too small particles
				if (p.scale < 0.f) continue;

				p.color = color_lerp(p.color, glm::vec4(1.f, 1.f, 0.f, 1.f), 1.f - p.lifespan / p.startingLife);

				particles.push_back(p);
			}
		}

#define EMISSION_LIFESPAN_MAX 0.3f
#define EMISSION_LIFESPAN_MIN 0.1f
#define EMISSION_SCALE_MAX 0.15f
#define EMISSION_SCALE_MIN 0.01f

#define EMISSION_MAX_VELOCITY_OFFSET_ANGLE 0.75f
#define EMISSION_SPEED_MAX 0.75f
#define EMISSION_SPEED_MIN 0.25f
#define EMISSION_ANGULAR_SPEED_MAX  10.f
#define EMISSION_ANGULAR_SPEED_MIN -10.f

#define EMISSION_SCALING_MAX 0.f
#define EMISSION_SCALING_MIN -0.5f

		//(float(rand()) / RAND_MAX)

		//new particle generation - only when the ball is moving
		if (!state.b.onPlatform) {
			Particle p;

			float theta_rad = atan2f(-state.b.dir.y, -state.b.dir.x);
			float _1_ballSpeed = 1.f / state.b.speed;

			for (int i = 0; i < 5; i++) {
				p.lifespan = p.startingLife = (float(rand()) / RAND_MAX) * (EMISSION_LIFESPAN_MAX - EMISSION_LIFESPAN_MIN) + EMISSION_LIFESPAN_MIN;
				p.position = state.b.pos;
				p.angle_rad = (float(rand()) / RAND_MAX) * float(M_PI) * 2.f;
				p.scale = (float(rand()) / RAND_MAX) * (EMISSION_SCALE_MAX - EMISSION_SCALE_MIN) + EMISSION_SCALE_MIN;

				float angle_rad = theta_rad + ((float(rand()) / RAND_MAX) * 2.f * M_PI - M_PI) * EMISSION_MAX_VELOCITY_OFFSET_ANGLE;
				p.velocity = glm::vec2(cosf(angle_rad), sinf(angle_rad)) * ((EMISSION_SPEED_MAX - EMISSION_SPEED_MIN) * (float(rand()) / RAND_MAX) + EMISSION_SPEED_MIN) * _1_ballSpeed;

				p.angularVelocity = (EMISSION_ANGULAR_SPEED_MAX - EMISSION_ANGULAR_SPEED_MIN) * (float(rand()) / RAND_MAX) + EMISSION_ANGULAR_SPEED_MIN;
				p.deltaScale = (EMISSION_SCALING_MAX - EMISSION_SCALING_MIN) * (float(rand()) / RAND_MAX) + EMISSION_SCALING_MIN;

				p.color = glm::vec4(1.f, 0.f, 0.f, 1.f) + glm::vec4(-0.3f * (float(rand()) / RAND_MAX), 0.3f * (float(rand()) / RAND_MAX), 0.f, 0.f);

				particles.push_back(p);
			}
		}

		//LOG(LOG_INFO, "Particle count: %d\n", (int)particles.size());
	}

}//namespace Game
