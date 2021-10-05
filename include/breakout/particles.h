#pragma once

#include "breakout/glm.h"

#include <vector>

struct Particle {
	glm::vec2 position;
	float angle_rad = 0.f;
	float scale = 1.f;


	glm::vec2 velocity = glm::vec2(0.f);
	float angularVelocity = 0.f;
	float deltaScale = 0.f;

	float lifespan = 0.f;
	float startingLife = 0.f;		//for color lerping purposes

	glm::vec4 color;
};

class ParticleSystem {
public:
	typedef void (*ParticleUpdateFn)(std::vector<Particle>& particles_prev, std::vector<Particle>& particles, float deltaTime);
public:
	ParticleSystem(ParticleUpdateFn ParticleUpdate, int maxCount = 10);
	ParticleSystem() = default;

	void Update(float deltaTime);
	void Render();

	void Reset();
private:
	std::vector<Particle> pbuf[2];
	int currentIdx = 0;

	ParticleUpdateFn ParticleUpdate = nullptr;
};