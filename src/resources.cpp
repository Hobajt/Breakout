#include "breakout/resources.h"

namespace Resources {

	std::map<std::string, ShaderRef> shaders;

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

}//namespace Resources
