#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (set = 0, binding = 4) uniform Transform { mat4 Projection; mat4 View; mat4 World; mat4 InverseViewProjection; };

layout (location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2D Sampler2D; //!< –@ü(Normal)

void main()
{
	OutColor = vec4(texture(Sampler2D, InTexcoord).rrr, 1.0f);
}