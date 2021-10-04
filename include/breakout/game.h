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
			Brick = 1,
			Wall = 2,
		};
	}//namespace BrickType

	struct Brick {
		int type = BrickType::Brick;
		glm::vec2 pos = glm::vec2(0.f);
		glm::ivec2 coords = glm::ivec2(0);
	public:
		Brick() = default;
		Brick(int x, int y, int type_) : coords(glm::ivec2(x, y)), type(type_) {}
	};

}//namespace Game
