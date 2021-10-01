#pragma once

#include "breakout/glm.h"
#include "breakout/shader.h"

struct Vertex {
	glm::vec3 position;
	glm::vec4 color;
	glm::vec2 texCoords;
	float textureID;
public:
	Vertex() = default;
	Vertex(const glm::vec3& position, const glm::vec4& color, const glm::vec2& texCoords, float textureID);
};

struct Quad {
	Vertex vertices[4];
public:
	Quad() = default;
	Quad(const glm::vec3& center, const glm::vec2& halfSize, const glm::vec4& color, float textureID);
	Quad(const glm::vec3& center, const glm::vec2& halfSize, const glm::vec4& color, float textureID, float angle_rad);
};

struct QuadIndices {
	uint32_t indices[5];
public:
	QuadIndices() = default;
	QuadIndices(int idx);
};

namespace Renderer {

	void SetShader(ShaderRef& shader);

	//Begins a rendering session.
	void Begin();

	//Ends rendering session (renders queued quads).
	void End();

	//Flushes all currently queued quads for rendering.
	//Called internally, whenever the buffers are full.
	//Triggers additional draw call.
	void Flush();

	void RenderQuad(const glm::vec3& center, const glm::vec2& halfSize, float textureID = 0.f);
	void RenderQuad(const glm::vec3& center, const glm::vec2& halfSize, const glm::vec4& color);

	void RenderRotatedQuad(const glm::vec3& center, const glm::vec2& halfSize, float angle_rad, float textureID = 0.f);
	void RenderRotatedQuad(const glm::vec3& center, const glm::vec2& halfSize, float angle_rad, const glm::vec4& color);

}//namespace Renderer
