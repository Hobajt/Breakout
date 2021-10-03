#pragma once

#include "breakout/glm.h"

namespace Game {

	//General initialization. Needs to be called before Run().
	void Init();

	//Initiates the game logic.
	void Run();

	void Play();

	//==== Structs ====

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

	struct Ball {
		glm::vec2 pos = glm::vec2(0.f);
		glm::vec2 dir;
		float speed;
		float radius = 0.1f;
	};

	namespace BrickType {
		enum {
			Brick = 1,
			Wall = 2,
		};
	}//namespace BrickType

	struct Brick {
		glm::ivec2 coords = glm::ivec2(0);
		glm::vec2 pos = glm::vec2(0.f);
		int type = BrickType::Brick;
	public:
		Brick() = default;
		Brick(int x, int y, int type_) : coords(glm::ivec2(x, y)), type(type_) {}
	};

}//namespace Game


