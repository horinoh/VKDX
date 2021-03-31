#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec3 InNormal;
layout (location = 2) in vec2 InTexcoord;

layout (location = 0) out vec3 OutNormal;
layout (location = 1) out vec2 OutTexcoord;

const float Scale = 0.005f;
//const float Scale = 0.5f;
//const float Scale = 1.0f;

//layout (set = 0, binding = 0) uniform Transform { mat4 Projection; mat4 View; };

void main()
{
	//const mat4 PV = Projection * View;
	//const mat4 PV =  View * Projection;

	//gl_Position = PV * vec4(InPosition, 1.0f);
	gl_Position = vec4(InPosition * Scale, 1.0f);
	OutNormal = InNormal;
	OutTexcoord = InTexcoord;
}
