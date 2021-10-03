#pragma once

#include "breakout/glm.h"
#include "breakout/texture.h"

#include <memory>
#include <string>

struct CharInfo {
	glm::ivec2 size;
	glm::ivec2 bearing;		//aka. offsets
	glm::ivec2 advance;

	//character's x-offset within the atlas texture
	float textureOffset;
};

class Font;
using FontRef = std::shared_ptr<Font>;

class Font {
public:
	Font(const std::string& filepath, int fontHeight = 48);

	Font() = default;
	~Font();

	//copy disabled
	Font(const Font&) = delete;
	Font& operator=(const Font&) = delete;

	//move enabled
	Font(Font&&) noexcept;
	Font& operator=(Font&&) noexcept;

	CharInfo& operator[](char c);
	const CharInfo& operator[](char c) const;

	CharInfo& GetChar(char c);
	const AtlasTextureRef& GetAtlasTexture() const { return atlas; }

	glm::vec2 AtlasSizeDenom() const { return atlasSizeDenom; }
private:
	void Load(int targetFontHeight);

	void Release() noexcept;
	void Move(Font&&) noexcept;
private:
	AtlasTextureRef atlas;
	CharInfo chars[128];
	int fontHeight;

	std::string name;
	std::string filepath;
	glm::vec2 atlasSizeDenom;
};