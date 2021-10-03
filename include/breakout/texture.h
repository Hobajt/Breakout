#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "breakout/glm.h"

struct TextureParams {
	GLenum wrapping = GL_CLAMP_TO_EDGE;
	GLenum filtering = GL_LINEAR;
};

class ITexture;
using ITextureRef = std::shared_ptr<ITexture>;

class Texture;
using TextureRef = std::shared_ptr<Texture>;

class SubTexture;
using SubTextureRef = std::shared_ptr<SubTexture>;

class AtlasTexture;
using AtlasTextureRef = std::shared_ptr<AtlasTexture>;

class ITexture {
public:
	ITexture() = default;
	ITexture(const std::string& name);

	//copy disabled
	ITexture(const ITexture&) = delete;
	ITexture& operator=(const ITexture&) = delete;

	//move enabled
	ITexture(ITexture&&) noexcept;
	ITexture& operator=(ITexture&&) noexcept;

	virtual void Bind(int slot) const = 0;
	static void Unbind(int slot);

	virtual glm::vec2 TexCoords(int i) const = 0;
protected:
	std::string name;
};

class SubTexture : public ITexture {
	friend class AtlasTexture;
public:
	SubTexture(Texture* atlas, const glm::ivec2& offset, const glm::ivec2& size, const std::string& name = "subtexture");
	SubTexture(AtlasTexture* atlas, const glm::ivec2& coords, const std::string& name = "subtexture");

	SubTexture() = default;

	SubTexture(SubTexture&&) noexcept;
	SubTexture& operator=(SubTexture&&) noexcept;

	virtual void Bind(int slot) const override;
	virtual glm::vec2 TexCoords(int i) const override;

	bool MatchingCoords(int x, int y) const;

	Texture* GetAtlas() { return atlas; }
private:
	void Move(SubTexture&&) noexcept;
private:
	Texture* atlas;

	glm::ivec2 offset = glm::ivec2(0);
	glm::ivec2 size = glm::ivec2(0);
	glm::ivec2 coords = glm::ivec2(0);

	glm::vec2 texCoords[4];
};

//===== Texture =====

class Texture : public ITexture {
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

	//move enabled
	Texture(Texture&&) noexcept;
	Texture& operator=(Texture&&) noexcept;

	virtual void Bind(int slot) const override;
	virtual glm::vec2 TexCoords(int i) const override;

	int Width() const { return width; }
	int Height() const { return height; }
private:
	void Release() noexcept;
	void Move(Texture&&) noexcept;
protected:
	GLuint handle = 0;

	int width;
	int height;
	GLenum internalFormat;
	TextureParams params;
};

//===== AtlasTexture =====

class AtlasTexture : public Texture {
public:
	AtlasTexture(int width, int height, GLenum internalFormat, const std::string& name);
	AtlasTexture(int width, int height, GLenum internalFormat, GLenum format, GLenum dtype, const std::string& name);

	AtlasTexture(const std::string& filepath, const std::string& configFilepath, const TextureParams& params = {});
	AtlasTexture(const std::string& filepath, const glm::ivec2& splitSize, const TextureParams& params = {});

	AtlasTexture() = default;
	~AtlasTexture();

	AtlasTexture(AtlasTexture&&) noexcept;
	AtlasTexture& operator=(AtlasTexture&&) noexcept;

	SubTextureRef operator[](const std::string& key);
	SubTextureRef operator()(int x, int y);
	SubTextureRef operator()(int x, int y, const std::string& key);

	SubTextureRef GetTexture(int x, int y);

	glm::ivec2 SplitSize() const { return splitSize; }
private:
	void Release() noexcept;
	void Move(AtlasTexture&&) noexcept;
private:
	std::vector<SubTextureRef> textures;
	std::map<std::string, glm::ivec2> textureMap;
	glm::ivec2 splitSize;
};
