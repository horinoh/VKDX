#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosition;
//layout (location = 3) in vec3 InNormal;

layout (location = 0) out vec4 OutColor;

void main()
{
	gl_Position = vec4(InPosition, 1.0f);
}
