#include "breakout/log.h"

#include "breakout/game.h"

int main(int argc, char** argv) {
	Game::Init();

	Game::Run();

	LOG(LOG_INFO, "Done.\n");
	return 0;
}
