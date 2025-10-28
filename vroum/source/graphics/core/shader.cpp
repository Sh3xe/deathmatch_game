#include "shader.hpp"

// #include "cmake_defines.hpp"
// #include "gldebug.hpp"
// #include "core/logger.hpp"

#include <GL/glew.h>
#include <fstream>
#include <iterator>

vv::Shader::Shader(const std::string& vs_path, const std::string &fs_path) {
	m_is_valid = true;
	uint32_t vs = 0, fs = 0;

	// compiles shaders
	if (vs_path != "")
		vs = compile_shader(vs_path, GL_VERTEX_SHADER);

	if (fs_path != "")
		fs = compile_shader(fs_path, GL_FRAGMENT_SHADER);

	// create program and link shaders
	m_id = glCreateProgram();

	if (vs)
		glAttachShader(m_id, vs);

	if (fs)
		glAttachShader(m_id, fs);

	glLinkProgram(m_id);

	// error handling
	int success;
	glGetProgramiv(m_id, GL_LINK_STATUS, &success);
	if (!success) {
		char info[512];
		glGetProgramInfoLog(m_id, 512, nullptr, info);
		// SD_ERROR("Can't link shader: ", info);
		m_is_valid = false;
	}

	// cleaning
	if (vs)
		glDeleteShader(vs);
	if (fs)
		glDeleteShader(fs);

}

vv::Shader::~Shader() {
	glDeleteProgram(m_id);
}

void vv::Shader::bind() {
	if (!m_is_valid)
		throw std::runtime_error("Can't use a unvalid shader");
	else
		glUseProgram(m_id);
}

void vv::Shader::unbind() {
	glUseProgram(0);
}

void vv::Shader::set_int(const std::string& name, int value) {
	glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void vv::Shader::set_float(const std::string& name, float value) {
	glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void vv::Shader::set_vec2(const std::string& name, float x, float y) {
	glUniform2f(glGetUniformLocation(m_id, name.c_str()), x, y);
}

void vv::Shader::set_vec3(const std::string& name, float x, float y, float z) {
	glUniform3f(glGetUniformLocation(m_id, name.c_str()), x, y, z);
}

void vv::Shader::set_vec4(const std::string& name, float x, float y, float z, float w) {
	glUniform4f(glGetUniformLocation(m_id, name.c_str()), x, y, z, w);
}

void vv::Shader::set_mat4(const std::string& name, float* matrix) {
	glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, matrix);
}

uint32_t vv::Shader::compile_shader(const std::string& path, uint32_t type) {

	std::fstream file{ path, std::ios::in };

	if(!file) {
		// SD_ERROR("Failed to open shader: ", path, "\n");
		return 0;
	}

	file << std::noskipws;
	std::string source{ std::istream_iterator<char>(file), std::istream_iterator<char>() };

	auto c_str_source = source.c_str();

	file.close();

	uint32_t shader = glCreateShader(type);
	glShaderSource(shader, 1, &c_str_source, nullptr);
	glCompileShader(shader);

	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infos[512];
		glGetShaderInfoLog(shader, 512, nullptr, infos);
		// SD_ERROR("Failed to compile shader : ", infos);
		m_is_valid = false;
		return 0;
	}

	return shader;

}