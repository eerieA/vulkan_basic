#version 450
#include "common.glsl"

layout(location = 0) in vec4 vertex_color;

layout(location = 0) out vec4 out_color;

void main() {
	out_color = vertex_color;
}