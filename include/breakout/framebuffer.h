#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "breakout/texture.h"

#include <memory>

class Framebuffer;
using FramebufferRef = std::shared_ptr<Framebuffer>;

class Framebuffer {
public:
	Framebuffer(int width, int height, GLenum internalFormat, const TextureParams& tParams = {});

	Framebuffer() = default;

	void Resize(int newWidth, int newHeight);

	void Bind() const;
	static void Unbind();

	TextureRef& GetTexture() { return texture; }

	bool IsComplete() const;
private:
	GLuint handle = 0;
	TextureRef texture = nullptr;

	int width;
	int height;
	GLenum internalFormat;

};