#include "breakout/renderer.h"

Vertex::Vertex(const glm::vec3& position_, const glm::vec4& color_, const glm::vec2& texCoords_, float textureID_) : position(position_), color(color_), texCoords(texCoords_), textureID(textureID_), texTiling(glm::vec2(1.f)) {}

Quad::Quad(const glm::vec3& center, const glm::vec2& halfSize, const glm::vec4& color) : Quad(center, halfSize, color, 0.f, nullptr) {}
Quad::Quad(const glm::vec3& center, const glm::vec2& halfSize, const glm::vec4& color, float angle_rad) : Quad(center, halfSize, color, 0.f, nullptr, angle_rad) {}

Quad::Quad(const glm::vec3& center, const glm::vec2& halfSize, float textureID, const ITextureRef& texture) : Quad(center, halfSize, glm::vec4(1.f), textureID, texture) {}
Quad::Quad(const glm::vec3& center, const glm::vec2& halfSize, float textureID, const ITextureRef& texture, float angle_rad) : Quad(center, halfSize, glm::vec4(1.f), textureID, texture, angle_rad) {}

Quad::Quad(const glm::vec3& center, const glm::vec2& hs, const glm::vec4& colorTint, float textureID, const ITextureRef& texture) {
	vertices[0] = Vertex(center + glm::vec3(-hs.x, -hs.y, 0.f), colorTint, glm::vec2(0.f, 1.f), textureID);
	vertices[1] = Vertex(center + glm::vec3(-hs.x, hs.y, 0.f), colorTint, glm::vec2(0.f, 0.f), textureID);
	vertices[2] = Vertex(center + glm::vec3(hs.x, -hs.y, 0.f), colorTint, glm::vec2(1.f, 1.f), textureID);
	vertices[3] = Vertex(center + glm::vec3(hs.x, hs.y, 0.f), colorTint, glm::vec2(1.f, 0.f), textureID);

	if (texture != nullptr) {
		vertices[0].texCoords = texture->TexCoords(0);
		vertices[1].texCoords = texture->TexCoords(1);
		vertices[2].texCoords = texture->TexCoords(2);
		vertices[3].texCoords = texture->TexCoords(3);
	}
}

Quad::Quad(const glm::vec3& center, const glm::vec2& hs, const glm::vec4& colorTint, float textureID, const ITextureRef& texture, float angle_rad) {
	float c = cosf(angle_rad);
	float s = sinf(angle_rad);
	glm::mat2 R = glm::mat2(
		glm::vec2(c, s),
		glm::vec2(-s, c)
	);
	glm::vec2 x = R * glm::vec2(hs.x, 0.f);
	glm::vec2 y = R * glm::vec2(0.f, hs.y);

	vertices[0] = Vertex(center + glm::vec3(-x - y, 0.f), colorTint, glm::vec2(0.f, 1.f), textureID);
	vertices[1] = Vertex(center + glm::vec3(-x + y, 0.f), colorTint, glm::vec2(0.f, 0.f), textureID);
	vertices[2] = Vertex(center + glm::vec3(x - y, 0.f), colorTint, glm::vec2(1.f, 1.f), textureID);
	vertices[3] = Vertex(center + glm::vec3(x + y, 0.f), colorTint, glm::vec2(1.f, 0.f), textureID);

	if (texture != nullptr) {
		vertices[0].texCoords = texture->TexCoords(0);
		vertices[1].texCoords = texture->TexCoords(1);
		vertices[2].texCoords = texture->TexCoords(2);
		vertices[3].texCoords = texture->TexCoords(3);
	}
}

QuadIndices::QuadIndices(int idx) {
	for(int i = 0; i < 4; i++)
		indices[i] = (idx * 4) + i;
	indices[4] = (uint32_t)-1;
}

namespace Renderer {

	constexpr int maxTextures = 8;

	float ResolveTextureIdx(const ITextureRef& texture);

	struct RendererData {
		GLuint vao = 0;
		GLuint vbo = 0;
		GLuint ebo = 0;

		Quad* quadsBuffer = nullptr;
		QuadIndices* indicesBuffer = nullptr;

		int batchSize = 1000;
		int idx = 0;

		ShaderRef shader = nullptr;
		bool inProgress = false;

		TextureRef blankTexture = nullptr;
		ITextureRef textures[maxTextures];
		int texIdx = 1;
	};

	static RendererData data;

	void SetShader(ShaderRef& shader) {
		data.shader = shader;
	}

	void Begin() {
		ASSERT_MSG(data.shader != nullptr, "\tRenderer - calling Begin() without a proper shader - set shader via SetShader() function.\n");

		if (data.inProgress) {
			LOG(LOG_WARN, "Renderer - Multiple Begin() calls without calling End().\n");
		}
		data.inProgress = true;

		//first call -> allocate resources
		if (data.quadsBuffer == nullptr) {
			//=== quad vertices & indices buffers ===
			data.quadsBuffer = new Quad[data.batchSize];
			data.indicesBuffer = new QuadIndices[data.batchSize];

			//=== their GPU counterparts ===
			glGenVertexArrays(1, &data.vao);
			glGenBuffers(1, &data.vbo);
			glGenBuffers(1, &data.ebo);

			glBindVertexArray(data.vao);

			glBindBuffer(GL_ARRAY_BUFFER, data.vbo);

			//proper attributes setup
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texTiling));
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textureID));
			glEnableVertexAttribArray(4);

			glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Quad) * data.batchSize, nullptr, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(QuadIndices) * data.batchSize, nullptr, GL_DYNAMIC_DRAW);

			//=== empty texture ===
			uint8_t tmp[] = { 255,255,255,255 };
			data.blankTexture = std::make_shared<Texture>(1, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, &tmp, "blankTexture");


			for (int i = 0; i < maxTextures; i++) {
				data.textures[i] = data.blankTexture;
			}

			//=== setup texture slots in shader ===
			data.shader->Bind();
			char buf[256];
			for (int i = 0; i < maxTextures; i++) {
				snprintf(buf, sizeof(buf), "textures[%d]", i);
				data.shader->SetInt(buf, i);
			}
		}

		data.idx = 0;
		data.texIdx = 1;
	}

	void End() {
		if (!data.inProgress) {
			LOG(LOG_WARN, "Renderer - Multiple End() calls without calling Begin().\n");
		}
		data.inProgress = false;

		Flush();
	}

	void Flush() {
		glEnable(GL_PRIMITIVE_RESTART);
		glPrimitiveRestartIndex((unsigned int)-1);

		data.shader->Bind();
		glBindVertexArray(data.vao);

		glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Quad) * data.idx, data.quadsBuffer);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ebo);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(QuadIndices) * data.idx, data.indicesBuffer);

		for (int i = 0; i < maxTextures; i++) {
			data.textures[i]->Bind(i);
		}

		glDrawElements(GL_TRIANGLE_STRIP, data.idx * 5, GL_UNSIGNED_INT, nullptr);

		data.idx = 0;
		data.texIdx = 1;
	}

	void RenderQuad(const glm::vec3& center, const glm::vec2& halfSize, const ITextureRef& texture) {
		float textureIdx = ResolveTextureIdx(texture);

		data.quadsBuffer[data.idx] = Quad(center, halfSize, textureIdx, texture);
		data.indicesBuffer[data.idx] = QuadIndices(data.idx);
		data.idx++;

		if (data.idx >= data.batchSize) {
			Flush();
		}
	}

	void RenderQuad(const glm::vec3& center, const glm::vec2& halfSize, const glm::vec4& color) {
		data.quadsBuffer[data.idx] = Quad(center, halfSize, color);
		data.indicesBuffer[data.idx] = QuadIndices(data.idx);
		data.idx++;

		if (data.idx >= data.batchSize) {
			Flush();
		}
	}

	void RenderRotatedQuad(const glm::vec3& center, const glm::vec2& halfSize, float angle_rad, const ITextureRef& texture) {
		float textureIdx = ResolveTextureIdx(texture);

		data.quadsBuffer[data.idx] = Quad(center, halfSize, textureIdx, texture, angle_rad);
		data.indicesBuffer[data.idx] = QuadIndices(data.idx);
		data.idx++;

		if (data.idx >= data.batchSize) {
			Flush();
		}
	}

	void RenderRotatedQuad(const glm::vec3& center, const glm::vec2& halfSize, float angle_rad, const glm::vec4& color) {
		data.quadsBuffer[data.idx] = Quad(center, halfSize, color, angle_rad);
		data.indicesBuffer[data.idx] = QuadIndices(data.idx);
		data.idx++;

		if (data.idx >= data.batchSize) {
			Flush();
		}
	}

	float ResolveTextureIdx(const ITextureRef& texture) {
		float idx = 0.f;

		//search for the texture in already queued textures
		for (int i = 0; i < maxTextures; i++) {
			if (data.textures[i] == texture) {
				idx = float(i);
				break;
			}
		}

		//not found, add texture into it's own slot
		if (idx == 0.f) {
			//trigger a draw call, if all the slots are already taken
			if (data.texIdx >= maxTextures) {
				Flush();
			}

			idx = data.texIdx;
			data.textures[data.texIdx] = texture;
			data.texIdx++;
		}

		return idx;
	}

}//namespace Renderer
