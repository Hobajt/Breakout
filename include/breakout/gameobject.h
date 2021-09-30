#pragma once

#include "breakout/glm.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoords;
};

struct Quad {
	Vertex vertices[4];
};