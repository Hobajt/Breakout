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

	ITextureRef GetTexture(const std::string& key);
	void AddTexture(const std::string& key, ITextureRef& shader);

	template<typename... Args>
	ITextureRef TryGetTexture(const std::string& key, Args&&... args) {
		ITextureRef result = GetTexture(key);

		if (result == nullptr) {
			result = std::make_shared<Texture>(std::forward<Args>(args)...);
			AddTexture(key, result);
		}

		return result;
	}

}//namespace Resources
