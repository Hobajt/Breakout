#include "breakout/shader.h"

#include "breakout/log.h"
#include "breakout/utils.h"


GLuint CompileSource(const char* source, GLenum shaderType);
GLuint LinkProgram(GLuint vertex, GLuint fragment);

//===== Shader =====

DBG_ONLY(GLuint Shader::activeProgram = 0);

#define SHADER_VALIDATION_CHECK() ASSERT_MSG(program != 0, "\tAttempting to use uninitialized shader object (%s).\n", name.c_str());
#define ACTIVE_SHADER_VALIDATION_CHECK() DBG_ONLY(ASSERT_MSG(activeProgram == program, "\tAttempting to use unbound shader (%s), use shader.Bind() first.\n", name.c_str()));

Shader::Shader(const std::string& filepath) : Shader(filepath + ".vert", filepath + ".frag") {}

Shader::Shader(const std::string& vertexFilepath, const std::string& fragmentFilepath) 
	: Shader(ReadFile(vertexFilepath.c_str()), ReadFile(fragmentFilepath.c_str()), fragmentFilepath.substr(0, fragmentFilepath.size() - 5)) {}

Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource, const std::string& name_) : name(name_) {
	GLuint vertex = CompileSource(vertexSource.c_str(), GL_VERTEX_SHADER);
	GLuint fragment = CompileSource(fragmentSource.c_str(), GL_FRAGMENT_SHADER);

	program = LinkProgram(vertex, fragment);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	LOG(LOG_RESOURCE, "Shader '%s' successfully compiled & linked.\n", name.c_str());
	LOG(LOG_CTOR, "[C] Shader '%s' (%d)\n", name.c_str(), program);
}

Shader::~Shader() {
	Release();
}

Shader::Shader(Shader&& s) noexcept {
	Move(std::move(s));
}

Shader& Shader::operator=(Shader&& s) noexcept {
	Release();
	Move(std::move(s));
	return *this;
}

void Shader::Bind() const {
	SHADER_VALIDATION_CHECK();
	DBG_ONLY(activeProgram = program);

	glUseProgram(program);
}

void Shader::Unbind() {
	DBG_ONLY(activeProgram = 0);

	glUseProgram(0);
}

void Shader::SetVec2(const char* varName, const glm::vec2& value) const {
	SHADER_VALIDATION_CHECK();
	ACTIVE_SHADER_VALIDATION_CHECK();
	glUniform2fv(glGetUniformLocation(program, varName), 1, (const GLfloat*)&value);
}

void Shader::SetVec3(const char* varName, const glm::vec3& value) const {
	SHADER_VALIDATION_CHECK();
	ACTIVE_SHADER_VALIDATION_CHECK();
	glUniform3fv(glGetUniformLocation(program, varName), 1, (const GLfloat*)&value);
}

void Shader::SetVec4(const char* varName, const glm::vec4& value) const {
	SHADER_VALIDATION_CHECK();
	ACTIVE_SHADER_VALIDATION_CHECK();
	glUniform4fv(glGetUniformLocation(program, varName), 1, (const GLfloat*)&value);
}

void Shader::SetMat2(const char* varName, const glm::mat2& value) const {
	SHADER_VALIDATION_CHECK();
	ACTIVE_SHADER_VALIDATION_CHECK();
	glUniformMatrix2fv(glGetUniformLocation(program, varName), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetMat3(const char* varName, const glm::mat3& value) const {
	SHADER_VALIDATION_CHECK();
	ACTIVE_SHADER_VALIDATION_CHECK();
	glUniformMatrix3fv(glGetUniformLocation(program, varName), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetMat4(const char* varName, const glm::mat4& value) const {
	SHADER_VALIDATION_CHECK();
	ACTIVE_SHADER_VALIDATION_CHECK();
	glUniformMatrix4fv(glGetUniformLocation(program, varName), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetInt(const char* varName, int value) const {
	SHADER_VALIDATION_CHECK();
	ACTIVE_SHADER_VALIDATION_CHECK();
	glUniform1i(glGetUniformLocation(program, varName), value);
}

void Shader::SetBool(const char* varName, bool value) const {
	SHADER_VALIDATION_CHECK();
	ACTIVE_SHADER_VALIDATION_CHECK();
	glUniform1i(glGetUniformLocation(program, varName), (int)value);
}

void Shader::SetFloat(const char* varName, float value) const {
	SHADER_VALIDATION_CHECK();
	ACTIVE_SHADER_VALIDATION_CHECK();
	glUniform1f(glGetUniformLocation(program, varName), value);
}

void Shader::SetUint(const char* varName, unsigned int value) const {
	SHADER_VALIDATION_CHECK();
	ACTIVE_SHADER_VALIDATION_CHECK();
	glUniform1ui(glGetUniformLocation(program, varName), value);
}

void Shader::SetARBHandle(const char* varName, uint64_t value) const {
	SHADER_VALIDATION_CHECK();
	ACTIVE_SHADER_VALIDATION_CHECK();
	glUniformHandleui64ARB(glGetUniformLocation(program, varName), value);
}

void Shader::Release() noexcept {
	if (program != 0) {
		LOG(LOG_DTOR, "[D] Shader '%s' (%d)\n", name.c_str(), program);
		glDeleteProgram(program);
		program = 0;
	}
}

void Shader::Move(Shader&& s) noexcept {
	program = s.program;
	name = s.name;

	s.program = 0;
}

//===== Utility functions =====

GLuint CompileSource(const char* source, GLenum shaderType) {
	const char* shaderTypeStr = (shaderType == GL_FRAGMENT_SHADER) ? "Fragment" : "Vertex";

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	int success;
	char infoLog[512];

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
		LOG(LOG_DEBUG, "Shader::CompileSource - %s shader failed to compile:\n%s", shaderTypeStr, infoLog);
		throw std::exception();
	}

	LOG(LOG_FINE, "%s shader successfully compiled.\n", shaderTypeStr);
	return shader;
}

GLuint LinkProgram(GLuint vertex, GLuint fragment) {
	GLuint shader = glCreateProgram();
	glAttachShader(shader, vertex);
	glAttachShader(shader, fragment);
	glLinkProgram(shader);

	int success;
	char infoLog[512];

	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader, sizeof(infoLog), NULL, infoLog);
		LOG(LOG_DEBUG, "Shader::LinkProgram - Program failed to link:\n%s", infoLog);
		glDeleteProgram(shader);
		throw std::exception();
	}

	LOG(LOG_FINE, "Shader successfully linked\n");
	return shader;
}