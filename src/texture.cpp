#include "breakout/texture.h"
#include "breakout/log.h"

#include <stb_image.h>

#define TEXTURE_VALIDATION_CHECK() ASSERT_MSG(handle != 0, "\tAttempting to use uninitialized texture (%s).\n", name.c_str())

void Texture::FlipOnLoad(bool flip) {
	stbi_set_flip_vertically_on_load(flip);
}

Texture::Texture(const std::string& filepath, const TextureParams& params_) : params(params_) {
	//load image data
	int channels;
	uint8_t* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
	if (data == nullptr) {
		LOG(LOG_WARN, "Failed to load texture from '%s'.\n", filepath.c_str());
		throw std::exception();
	}

	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);

	if (params.wrapping != GL_NONE) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params.wrapping);
	}
	if (params.filtering != GL_NONE) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.filtering);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.filtering);
	}

	GLenum format;
	switch (channels) {
		case 1:		
			format = GL_RED;  
			break;
		default:
		case 3:		
			format = GL_RGB;  
			break;
		case 4:		
			format = GL_RGBA; 
			break;
	}
	internalFormat = format;		//TODO: change, if doing SRGB conversion

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	//name resolution
	size_t pos = filepath.find_last_of("/\\");
	if (pos != std::string::npos) {
		name = filepath.substr(pos + 1);
	}
	else {
		name = filepath;
	}

	LOG(LOG_RESOURCE, "Loaded texture from '%s'.\n", name.c_str());
	LOG(LOG_CTOR, "[C] Texture '%s' (%d)\n", name.c_str(), handle);
}

Texture::Texture(int width_, int height_, GLenum internalFormat_, const std::string& name_, const TextureParams& params_)
	: Texture(width_, height_, internalFormat_, GL_RGB, GL_UNSIGNED_BYTE, nullptr, params_, name_) {}

Texture::Texture(int width_, int height_, GLenum internalFormat_, const TextureParams& params_, const std::string& name_)
	: Texture(width_, height_, internalFormat_, GL_RGB, GL_UNSIGNED_BYTE, nullptr, params_, name_) {}

Texture::Texture(int width_, int height_, GLenum internalFormat_, GLenum format, GLenum dtype, void* data, const std::string& name_, const TextureParams& params_)
	: Texture(width_, height_, internalFormat_, format, dtype, data, params_, name_) {}

Texture::Texture(int width_, int height_, GLenum internalFormat_, GLenum format, GLenum dtype, void* data, const TextureParams& params_, const std::string& name_)
	: width(width_), height(height_), internalFormat(internalFormat_), params(params_), name(name_) {
	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dtype, data);

	glBindTexture(GL_TEXTURE_2D, 0);

	LOG(LOG_RESOURCE, "Created custom texture '%s' (%dx%d).\n", name.c_str(), width, height);
	LOG(LOG_CTOR, "[C] Texture '%s' (%d)\n", name.c_str(), handle);
}

Texture::~Texture() {
	Release();
}

Texture::Texture(Texture&& t) noexcept {
	Move(std::move(t));
}

Texture& Texture::operator=(Texture&& t) noexcept {
	Release();
	Move(std::move(t));
	return *this;
}

void Texture::Bind(int slot) const {
	TEXTURE_VALIDATION_CHECK();

	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, handle);
}

void Texture::Unbind(int slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Release() noexcept {
	if (handle != 0) {
		LOG(LOG_DTOR, "[D] Texture '%s' (%d)\n", name.c_str(), handle);

		glDeleteTextures(1, &handle);
		handle = 0;
	}
}

void Texture::Move(Texture&& t) noexcept {
	handle = t.handle;
	name = t.name;
	params = t.params;
	width = t.width;
	height = t.height;
	internalFormat = t.internalFormat;

	t.handle = 0;
}
