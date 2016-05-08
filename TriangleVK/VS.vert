#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 Position;
layout (location = 1) in vec4 Color;

out OUT
{
	vec4 Position;
	vec4 Color;
} Out;

void main()
{
	Out.Position = vec4(Position, 1.0f);
	Out.Color = Color;
}
