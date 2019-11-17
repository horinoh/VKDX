#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 InColor;
layout (location = 1) in vec3 InPosition;
layout (location = 2) in vec3 InNormal;
layout (location = 3) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec3 OutNormal;
layout (location = 2) out vec2 OutTexcoord;


const float Scale = 1.0f;

void main()
{
	OutColor = InColor;
	gl_Position = vec4(InPosition * Scale, 1.0f);
	OutNormal = InNormal;
	OutTexcoord = InTexcoord;
}
