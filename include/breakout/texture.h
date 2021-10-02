#pragma once

#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Texture;
using TextureRef = std::shared_ptr<Texture>;

struct TextureParams {
	GLenum wrapping = GL_CLAMP_TO_EDGE;
	GLenum filtering = GL_LINEAR;
};

class Texture {
public:
	static void FlipOnLoad(bool flip);
public:
	Texture(const std::string& filepath, const TextureParams& params = {});

	Texture(int width, int height, GLenum internalFormat, const std::string& name, const TextureParams& params = {});
	Texture(int width, int height, GLenum internalFormat, const TextureParams& params = {}, const std::string& name = "custom");

	Texture(int width, int height, GLenum internalFormat, GLenum format, GLenum dtype, void* data, const std::string& name, const TextureParams& params = {});
	Texture(int width, int height, GLenum internalFormat, GLenum format, GLenum dtype, void* data, const TextureParams& params = {}, const std::string& name = "custom");

	Texture() = default;
	~Texture();

	//copy disabled
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	//move enabled
	Texture(Texture&&) noexcept;
	Texture& operator=(Texture&&) noexcept;

	void Bind(int slot) const;
	static void Unbind(int slot);
private:
	void Release() noexcept;
	void Move(Texture&&) noexcept;
private:
	GLuint handle = 0;
	std::string name;

	int width;
	int height;
	GLenum internalFormat;
	TextureParams params;
};
