#pragma once

#include "vv_headers.hpp"
#include <string>

namespace vv
{

class Shader {
public:
	Shader(const std::string& vs_path, const std::string& fs_path);
	~Shader();

	void bind();
	void unbind();

	operator bool() const { return m_is_valid; }

	void set_int( const std::string &name, int value );
	void set_float( const std::string& name, float value);
	void set_vec2( const std::string& name, float x, float y);
	void set_vec3( const std::string& name, float x, float y, float z);
	void set_vec4( const std::string& name, float x, float y, float z, float w);
	void set_mat4( const std::string& name, float* matrix );

private:
	vv::u32 compile_shader( const std::string &path, vv::u32 type);

	vv::u32 m_id;
	bool m_is_valid = false;
};

} // namespace vv