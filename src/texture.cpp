#include "breakout/texture.h"
#include "breakout/log.h"

#include <stb_image.h>

//===== ITexture =====

ITexture::ITexture(const std::string& name_) : name(name_) {}

ITexture::ITexture(ITexture&& t) noexcept {
	name = t.name;
}

ITexture& ITexture::operator=(ITexture&& t) noexcept {
	name = t.name;
	return *this;
}

void ITexture::Unbind(int slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//===== SubTexture =====

#define SUBTEXTURE_VALIDATION_CHECK() ASSERT_MSG(atlas != nullptr, "\tAttempting to use uninitialized subtexture (%s).\n", name.c_str())

SubTexture::SubTexture(Texture* atlas_, const glm::ivec2& offset_, const glm::ivec2& size_, const std::string& name_) : ITexture(name_), atlas(atlas_), offset(offset_), size(size_), coords(glm::ivec2(0)) {
	glm::vec2 _1_atlasSize = 1.f / glm::vec2(atlas->Width(), atlas->Height());
	glm::vec2 of = glm::vec2(offset);

	texCoords[0] = of + glm::vec2(0.f, 0.f) * _1_atlasSize;
	texCoords[1] = of + glm::vec2(0.f, size.y) * _1_atlasSize;
	texCoords[2] = of + glm::vec2(size.x, 0.f) * _1_atlasSize;
	texCoords[3] = of + glm::vec2(size.x, size.y) * _1_atlasSize;
}

SubTexture::SubTexture(AtlasTexture* atlas_, const glm::ivec2& coords_, const std::string& name_) : ITexture(name_), atlas(atlas_), coords(coords_) {
	glm::vec2 atlasSize = glm::vec2(atlas->Width(), atlas->Height());

	size = atlas_->SplitSize();
	offset = size * coords;
	glm::vec2 of = glm::vec2(offset);

	texCoords[0] = (of + glm::vec2(0.f, size.y)) / atlasSize;
	texCoords[1] = (of + glm::vec2(0.f, 0.f)) / atlasSize;
	texCoords[2] = (of + glm::vec2(size.x, size.y)) / atlasSize;
	texCoords[3] = (of + glm::vec2(size.x, 0.f)) / atlasSize;
}

SubTexture::SubTexture(SubTexture&& st) noexcept : ITexture(std::move(st)) {
	Move(std::move(st));
}

SubTexture& SubTexture::operator=(SubTexture&& st) noexcept {
	ITexture::operator=(std::move(st));
	Move(std::move(st));
	return *this;
}

void SubTexture::Bind(int slot) const {
	SUBTEXTURE_VALIDATION_CHECK();
	atlas->Bind(slot);
}

glm::vec2 SubTexture::TexCoords(int i) const {
	return texCoords[i];
}

bool SubTexture::MatchingCoords(int x, int y) const {
	return (x == coords.x && y == coords.y);
}

void SubTexture::Move(SubTexture&& st) noexcept {
	atlas = st.atlas;
	offset = st.offset;
	coords = st.coords;
	size = st.size;
	for (int i = 0; i < 4; i++) {
		texCoords[i] = st.texCoords[i];
	}

	st.atlas = nullptr;
}

//===== Texture =====

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
	: ITexture(name_), width(width_), height(height_), internalFormat(internalFormat_), params(params_) {
	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dtype, data);
	if (params.filtering != GL_NONE) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.filtering);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.filtering);
	}
	if (params.wrapping != GL_NONE) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params.wrapping);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	LOG(LOG_RESOURCE, "Created custom texture '%s' (%dx%d).\n", name.c_str(), width, height);
	LOG(LOG_CTOR, "[C] Texture '%s' (%d)\n", name.c_str(), handle);
}

Texture::~Texture() {
	Release();
}

Texture::Texture(Texture&& t) noexcept : ITexture(std::move(t)) {
	Move(std::move(t));
}

Texture& Texture::operator=(Texture&& t) noexcept {
	Release();
	ITexture::operator=(std::move(t));
	Move(std::move(t));
	return *this;
}

void Texture::Bind(int slot) const {
	TEXTURE_VALIDATION_CHECK();

	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, handle);
}

glm::vec2 Texture::TexCoords(int i) const {
	switch (i) {
		case 0: return glm::vec2(0.f, 1.f);
		case 1: return glm::vec2(0.f, 0.f);
		case 2: return glm::vec2(1.f, 1.f);
		case 3: return glm::vec2(1.f, 0.f);
	}
	return glm::vec2(0.f);
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
	params = t.params;
	width = t.width;
	height = t.height;
	internalFormat = t.internalFormat;

	t.handle = 0;
}

//===== AtlasTexture =====

AtlasTexture::AtlasTexture(int width_, int height_, GLenum internalFormat_, const std::string& name_) : Texture(width_, height_, internalFormat_, name_), splitSize(glm::ivec2(0)) {}

AtlasTexture::AtlasTexture(int width_, int height_, GLenum internalFormat_, GLenum format_, GLenum dtype_, const std::string& name_) : Texture(width_, height_, internalFormat_, format_, dtype_, nullptr, name_), splitSize(glm::ivec2(0)) {}

AtlasTexture::AtlasTexture(const std::string& filepath, const std::string& configFilepath, const TextureParams& params) : AtlasTexture(filepath, glm::ivec2(0), params) {
	//TODO: load subtextures from config file
}

AtlasTexture::AtlasTexture(const std::string& filepath, const glm::ivec2& splitSize_, const TextureParams& params) : Texture(filepath, params), splitSize(splitSize_) {}

AtlasTexture::~AtlasTexture() {
	Release();
}

AtlasTexture::AtlasTexture(AtlasTexture&& at) noexcept : Texture(std::move(at)) {
	Move(std::move(at));
}

AtlasTexture& AtlasTexture::operator=(AtlasTexture&& at) noexcept {
	Release();
	Texture::operator=(std::move(at));
	Move(std::move(at));
	return *this;
}

SubTextureRef AtlasTexture::operator[](const std::string& key) {
	if (textureMap.count(key) != 0) {
		glm::ivec2 c = textureMap[key];
		return operator()(c.x, c.y);
	}
	else
		return nullptr;
}

SubTextureRef AtlasTexture::operator()(int x, int y) {
	//search cached subtextures
	for (auto& tex : textures) {
		if (tex->MatchingCoords(x, y))
			return tex;
	}

	int ex = x * splitSize.x;
	int ey = y * splitSize.y;
	ASSERT_MSG((x >= 0 && y >= 0 && ex < width && ey < height), "\tAtlasTexture - Accessing sub-texture out of atlas bounds.\n");

	//generate new subtexture if within bounds
	textures.emplace_back(std::make_shared<SubTexture>(this, glm::ivec2(x,y)));
	return textures.back();
}

SubTextureRef AtlasTexture::operator()(int x, int y, const std::string& key) {
	//search cached subtextures
		for (auto& tex : textures) {
			if (tex->MatchingCoords(x, y))
				return tex;
		}

	int ex = x * splitSize.x;
	int ey = y * splitSize.y;
	ASSERT_MSG((x < 0 || y < 0 || ex >= width || ey >= height), "\tAtlasTexture - Accessing sub-texture out of atlas bounds.\n");

	//generate new subtexture if within bounds
	textures.emplace_back(std::make_shared<SubTexture>(this, glm::ivec2(x, y), key));
	textureMap[key] = glm::ivec2(x, y);
	return textures.back();
}

SubTextureRef AtlasTexture::GetTexture(int x, int y) {
	return operator()(x, y);
}

void AtlasTexture::Release() noexcept {
	for (SubTextureRef& tex : textures) {
		tex->atlas = nullptr;
	}
	textures.clear();
}

void AtlasTexture::Move(AtlasTexture&& at) noexcept {
	textures = at.textures;
	textureMap = at.textureMap;
	splitSize = at.splitSize;
}
