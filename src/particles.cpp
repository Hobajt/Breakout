#include "breakout/particles.h"
#include "breakout/renderer.h"

ParticleSystem::ParticleSystem(ParticleUpdateFn ParticleUpdate_, int maxCount) : ParticleUpdate(ParticleUpdate_) {
	pbuf[0].reserve(maxCount);
	pbuf[1].reserve(maxCount);
}

void ParticleSystem::Update(float deltaTime) {
	ParticleUpdate(pbuf[currentIdx], pbuf[1 - currentIdx], deltaTime);
	currentIdx = 1 - currentIdx;
}

void ParticleSystem::Render() {
	for (const Particle& p : pbuf[currentIdx]) {
		if (fabsf(p.angle_rad) < 1e-3f)
			Renderer::RenderQuad(glm::vec3(p.position, 0.f), glm::vec2(p.scale * 0.1f), p.color);
		else
			Renderer::RenderRotatedQuad(glm::vec3(p.position, 0.f), glm::vec2(p.scale * 0.1f), p.angle_rad, p.color);
		//Renderer::RenderRotatedQuad(glm::vec3(p.position, 0.f), glm::vec2(p.scale * 0.1f), p.angle_rad, p.color);
	}
}

void ParticleSystem::Reset() {
	pbuf[0].clear();
	pbuf[1].clear();
	currentIdx = 0;
}