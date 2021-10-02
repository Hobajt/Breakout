#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "breakout/shader.h"
#include "breakout/texture.h"

namespace Resources {

	//==== Shader ====

	ShaderRef GetShader(const std::string& key);
	void AddShader(const std::string& key, ShaderRef& shader);

	template<typename... Args>
	ShaderRef TryGetShader(const std::string& key, Args&&... args) {
		ShaderRef result = GetShader(key);

		if (result == nullptr) {
			result = std::make_shared<Shader>(std::forward<Args>(args)...);
			AddShader(key, result);
		}

		return result;
	}

	//==== Texture ====

	TextureRef GetTexture(const std::string& key);
	void AddTexture(const std::string& key, TextureRef& shader);

	template<typename... Args>
	TextureRef TryGetTexture(const std::string& key, Args&&... args) {
		TextureRef result = GetTexture(key);

		if (result == nullptr) {
			result = std::make_shared<Texture>(std::forward<Args>(args)...);
			AddTexture(key, result);
		}

		return result;
	}

}//namespace Resources
