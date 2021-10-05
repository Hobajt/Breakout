#pragma once

#include "breakout/glm.h"

namespace Game {

	//Initiates the game logic.
	void Run();

	//==== Structs ====

	enum class GameState {
		MainMenu,
		Playing,
		Paused,
		IngameMenu,
		EndScreen,
		Transition,
	};

	struct Platform {
		float pos = 0.f;
		float scale;
		float speed;
	};

	struct Ball {
		glm::vec2 pos = glm::vec2(0.f);
		glm::vec2 dir = glm::vec2(0.f, 1.f);
		float speed;
		float radius = 0.05f;

		bool onPlatform = true;
	};

	namespace BrickType {
		enum {
			Brick = 0,
			Wall,
			PlatformGrow,
			PlatformShrink,
			PlatformSticking,
			WallBreaker,
			BallSpeedUp,
			BallSlowDown,
			EffectBlur,
			EffectDrunk,
			EffectChaos,
			EffectConfuse,
		};

		const char* BrickCodes();
		glm::ivec2 GetTypeTexCoord(int type, int color);
	}

	struct Brick {
		int type = BrickType::Brick;
		int color = 1;

		glm::vec2 pos = glm::vec2(0.f);
		glm::ivec2 coords = glm::ivec2(0);

		glm::ivec2 tc;
	public:
		Brick() = default;
		Brick(int x, int y, int type_, int color_) : coords(glm::ivec2(x, y)), type(type_), color(color_) {
			tc = BrickType::GetTypeTexCoord(type, color);
		}
		//Brick(int x, int y, char c) : coords(glm::ivec2(x, y)), type(int(c)), brickType(1) {}
	};

}//namespace Game
