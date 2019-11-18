#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in uvec4 InJoints;
layout (location = 1) in vec3 InPosition;
layout (location = 2) in vec3 InNormal;
layout (location = 3) in vec2 InTexcoord;
layout (location = 4) in vec4 InWeights;

layout (location = 0) out uvec4 OutJoints;
layout (location = 1) out vec3 OutNormal;
layout (location = 2) out vec2 OutTexcoord;
layout (location = 3) out vec4 OutWeights;

const float Scale = 0.5f;
//const float Scale = 0.02f;

void main()
{
	OutJoints = InJoints;
	gl_Position = vec4(InPosition * Scale, 1.0f);
	OutNormal = InNormal;
	OutTexcoord = InTexcoord;
	OutWeights = InWeights;
}
