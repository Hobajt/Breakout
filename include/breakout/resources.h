#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "breakout/shader.h"

namespace Resources {

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

}//namespace Resources
