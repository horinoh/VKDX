#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//!< Per Vertex
layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec4 InColor;

//!< Per Instance
layout (location = 2) in vec2 InOffset;

layout (location = 0) out vec4 OutColor;

void main()
{
	gl_Position = vec4(InPosition, 1.0f) + vec4(InOffset.x, InOffset.y, 0.0f, 0.0f);
	OutColor = InColor;

	//const vec4 Colors[] = { vec4(1, 0, 0, 1), vec4(0, 1, 0, 1), vec4(0, 0, 1, 1), vec4(1, 1, 0, 1), vec4(0, 1, 1, 1) };
	//OutColor = Colors[gl_InstanceIndex];
}
