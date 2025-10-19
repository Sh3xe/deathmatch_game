#version 450

layout (location=0) out vec4 frag_col;

layout (location=0) in vec3 vertex_col;

void main()
{
	frag_col = vec4(vertex_col, 1.0);
}