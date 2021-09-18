#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec3 InNormal;

layout (location = 0) out vec3 OutColor;

void main()
{
	const mat4 PVW = mat4(1.93643117f, 0.0f, 0.0f, 0.0f,
							0.0f, 3.89474273f, 0.0f, 0.0f,
							0.0f, 0.0f, -1.00010002f, -1.0f,
							0.0f, 0.0f, 2.99029899f, 3.0f);
	gl_Position = PVW * vec4(InPosition, 1.0f);
	OutColor = InNormal * 0.5f + 0.5f;
}
