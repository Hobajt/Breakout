#include "breakout/resources.h"

namespace Resources {

	std::map<std::string, ShaderRef> shaders;
	std::map<std::string, ITextureRef> textures;

	//==== Getters ====

	ShaderRef GetShader(const std::string& key) {
		if (shaders.count(key) > 0)
			return shaders.at(key);
		else
			return nullptr;
	}

	void AddShader(const std::string& key, ShaderRef& shader) {
		shaders[key] = shader;
	}

	ITextureRef GetTexture(const std::string& key) {
		if (textures.count(key) > 0)
			return textures.at(key);
		else
			return nullptr;
	}

	void AddTexture(const std::string& key, ITextureRef& texture) {
		textures[key] = texture;
	}

}//namespace Resources
