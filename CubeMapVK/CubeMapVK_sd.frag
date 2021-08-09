#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 2) in vec3 InPosition;

layout (binding = 1, set = 0) uniform samplerCube CubeMap;

layout (location = 0) out vec4 Color;

layout (early_fragment_tests) in;
void main()
{
	Color = texture(CubeMap, InPosition.xyz);
}