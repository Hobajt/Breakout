#include "breakout/text.h"

#include "breakout/log.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H

Font::Font(const std::string& filepath_, int fontHeight_) : filepath(filepath_), fontHeight(fontHeight_) {
	//name resolution
	size_t pos = filepath.find_last_of("/\\");
	if (pos != std::string::npos) {
		name = filepath.substr(pos + 1);
	}
	else {
		name = filepath;
	}

	Load(fontHeight);
}

Font::~Font() {
	Release();
}

Font::Font(Font&& f) noexcept {
	Move(std::move(f));
}

Font& Font::operator=(Font&& f) noexcept {
	Release();
	Move(std::move(f));
	return *this;
}

CharInfo& Font::operator[](char c) {
	return chars[int(c)];
}

const CharInfo& Font::operator[](char c) const {
	return chars[int(c)];
}

CharInfo& Font::GetChar(char c) {
	return operator[](c);
}

void Font::Load(int targetFontHeight) {
	FT_Library ft;
	FT_Face face;

	//library initialization
	if (FT_Init_FreeType(&ft)) {
		LOG(LOG_ERROR, "FreeType - Initialization failed.\n");
		throw std::exception();
	}

	//load font data
	if (FT_New_Face(ft, filepath.c_str(), 0, &face)) {
		LOG(LOG_ERROR, "FreeType - Font failed to load.\n");
		throw std::exception();
	}

	//set font size
	FT_Set_Pixel_Sizes(face, 0, targetFontHeight);

	unsigned int atlasWidth = 0;
	unsigned int atlasHeight = 0;

	unsigned char char_start = 32;
	unsigned char char_end = 128;
	FT_GlyphSlot g = face->glyph;

	//compute texture atlas dimensions (all glyphs will be in 1 row)
	for (unsigned char c = char_start; c < char_end; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			LOG(LOG_DEBUG, "FreeType - Glyph '%c' failed to load.\n", c);
			continue;
		}

		atlasWidth += g->bitmap.width;
		atlasHeight = std::max(atlasHeight, face->glyph->bitmap.rows);
	}

	//initialize atlas
	atlas = std::make_shared<AtlasTexture>(atlasWidth, atlasHeight, GL_RED, std::string("atlas_") + name);
	atlas->Bind(0);
	atlasSizeDenom = glm::vec2(1.f / atlasWidth, 1.f / atlasHeight);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//load each glyph into the texture
	int x = 0;
	for (unsigned char c = char_start; c < char_end; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			continue;
		}

		glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

		char buf[256];
		snprintf(buf, sizeof(buf), "char_%c", c);
		std::string texName = std::string(buf);

		chars[c] = CharInfo{
			glm::ivec2(g->bitmap.width, g->bitmap.rows),
			glm::ivec2(g->bitmap_left, g->bitmap_top),
			glm::ivec2(g->advance.x >> 6, g->advance.y >> 6),
			float(x)
		};
		

		x += g->bitmap.width;
	}

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
	Texture::Unbind(0);

	//library cleanup
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void Font::Release() noexcept {

}

void Font::Move(Font&& f) noexcept {

}