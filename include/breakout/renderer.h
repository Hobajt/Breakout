#pragma once

#include "breakout/glm.h"
#include "breakout/shader.h"
#include "breakout/texture.h"
#include "breakout/text.h"

struct Vertex {
	glm::vec3 position;
	glm::vec4 color;
	glm::vec2 texCoords;
	glm::vec2 texTiling;
	float textureID;
	float alphaTexture;
public:
	Vertex() = default;
	Vertex(const glm::vec3& position, const glm::vec4& color, const glm::vec2& texCoords, float textureID);
};

struct Quad {
	Vertex vertices[4];
public:
	Quad() = default;

	//color only quad
	Quad(const glm::vec3& center, const glm::vec2& halfSize, const glm::vec4& color);
	Quad(const glm::vec3& center, const glm::vec2& halfSize, const glm::vec4& color, float angle_rad);

	//texture only quad
	Quad(const glm::vec3& center, const glm::vec2& halfSize, float textureID, const ITextureRef& texture);
	Quad(const glm::vec3& center, const glm::vec2& halfSize, float textureID, const ITextureRef& texture, float angle_rad);

	//both
	Quad(const glm::vec3& center, const glm::vec2& halfSize, const glm::vec4& colorTint, float textureID, const ITextureRef& texture);
	Quad(const glm::vec3& center, const glm::vec2& halfSize, const glm::vec4& colorTint, float textureID, const ITextureRef& texture, float angle_rad);

	Quad(const CharInfo& charInfo, const glm::vec2& topLeft, float scale, const glm::vec4& color, float textureID, const glm::vec2& _1_atlSize, const glm::vec2& _1_winSize);
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

	void RenderQuad(const glm::vec3& center, const glm::vec2& halfSize, const ITextureRef& texture);
	void RenderQuad(const glm::vec3& center, const glm::vec2& halfSize, const glm::vec4& color);

	void RenderRotatedQuad(const glm::vec3& center, const glm::vec2& halfSize, float angle_rad, const ITextureRef& texture);
	void RenderRotatedQuad(const glm::vec3& center, const glm::vec2& halfSize, float angle_rad, const glm::vec4& color);

	void RenderText(const FontRef& font, const char* text, const glm::vec2& topLeft, float scale, const glm::vec4& color);
	void RenderText_Centered(const FontRef& font, const char* text, const glm::vec2& center, float scale, const glm::vec4& color);

}//namespace Renderer
