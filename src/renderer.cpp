#include "breakout/renderer.h"

Vertex::Vertex(const glm::vec3& position_, const glm::vec4& color_, const glm::vec2& texCoords_, float textureID_) : position(position_), color(color_), texCoords(texCoords_), textureID(textureID_) {}

Quad::Quad(const glm::vec3& center, const glm::vec2& hs, const glm::vec4& color, float textureID) {
	vertices[0] = Vertex(center + glm::vec3(-hs.x, -hs.y, 0.f), color, glm::vec2(0.f, 1.f), textureID);
	vertices[1] = Vertex(center + glm::vec3(-hs.x, hs.y, 0.f), color, glm::vec2(0.f, 0.f), textureID);
	vertices[2] = Vertex(center + glm::vec3(hs.x, -hs.y, 0.f), color, glm::vec2(1.f, 1.f), textureID);
	vertices[3] = Vertex(center + glm::vec3(hs.x, hs.y, 0.f), color, glm::vec2(1.f, 0.f), textureID);
}

Quad::Quad(const glm::vec3& center, const glm::vec2& hs, const glm::vec4& color, float textureID, float angle_rad) {
	float c = cosf(angle_rad);
	float s = sinf(angle_rad);
	glm::mat2 R = glm::mat2(
		glm::vec2( c, s),
		glm::vec2(-s, c)
	);
	glm::vec2 x = R * glm::vec2(hs.x, 0.f);
	glm::vec2 y = R * glm::vec2(0.f, hs.y);


	vertices[0] = Vertex(center + glm::vec3(-x - y, 0.f), color, glm::vec2(0.f, 1.f), textureID);
	vertices[1] = Vertex(center + glm::vec3(-x + y, 0.f), color, glm::vec2(0.f, 0.f), textureID);
	vertices[2] = Vertex(center + glm::vec3( x - y, 0.f), color, glm::vec2(1.f, 1.f), textureID);
	vertices[3] = Vertex(center + glm::vec3( x + y, 0.f), color, glm::vec2(1.f, 0.f), textureID);
}

QuadIndices::QuadIndices(int idx) {
	for(int i = 0; i < 4; i++)
		indices[i] = (idx * 4) + i;
	indices[4] = (uint32_t)-1;
}

namespace Renderer {

	struct RendererData {
		GLuint vao = 0;
		GLuint vbo = 0;
		GLuint ebo = 0;

		Quad* quadsBuffer;
		QuadIndices* indicesBuffer;

		int batchSize = 1000;
		int idx = 0;

		ShaderRef shader = nullptr;
		bool inProgress = false;
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
			data.quadsBuffer = new Quad[data.batchSize];
			data.indicesBuffer = new QuadIndices[data.batchSize];

			glGenVertexArrays(1, &data.vao);
			glGenBuffers(1, &data.vbo);
			glGenBuffers(1, &data.ebo);

			glBindVertexArray(data.vao);

			glBindBuffer(GL_ARRAY_BUFFER, data.vbo);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textureID));
			glEnableVertexAttribArray(3);

			glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(Quad) * data.batchSize, nullptr, GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(QuadIndices) * data.batchSize, nullptr, GL_DYNAMIC_DRAW);
		}

		data.idx = 0;
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

		glDrawElements(GL_TRIANGLE_STRIP, data.idx * 5, GL_UNSIGNED_INT, nullptr);

		data.idx = 0;
	}

	void RenderQuad(const glm::vec3& center, const glm::vec2& halfSize, float textureID) {
		data.quadsBuffer[data.idx] = Quad(center, halfSize, glm::vec4(1.f), textureID);
		data.indicesBuffer[data.idx] = QuadIndices(data.idx);
		data.idx++;

		if (data.idx >= data.batchSize) {
			Flush();
		}
	}

	void RenderQuad(const glm::vec3& center, const glm::vec2& halfSize, const glm::vec4& color) {
		data.quadsBuffer[data.idx] = Quad(center, halfSize, color, 0.f);
		data.indicesBuffer[data.idx] = QuadIndices(data.idx);
		data.idx++;

		if (data.idx >= data.batchSize) {
			Flush();
		}
	}

	void RenderRotatedQuad(const glm::vec3& center, const glm::vec2& halfSize, float angle_rad, float textureID) {
		data.quadsBuffer[data.idx] = Quad(center, halfSize, glm::vec4(1.f), textureID, angle_rad);
		data.indicesBuffer[data.idx] = QuadIndices(data.idx);
		data.idx++;

		if (data.idx >= data.batchSize) {
			Flush();
		}
	}

	void RenderRotatedQuad(const glm::vec3& center, const glm::vec2& halfSize, float angle_rad, const glm::vec4& color) {
		data.quadsBuffer[data.idx] = Quad(center, halfSize, color, 0.f, angle_rad);
		data.indicesBuffer[data.idx] = QuadIndices(data.idx);
		data.idx++;

		if (data.idx >= data.batchSize) {
			Flush();
		}
	}

}//namespace Renderer
