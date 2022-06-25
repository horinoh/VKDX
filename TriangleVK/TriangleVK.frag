#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//!< GLSL ‚Å #include ‚ªŽg‚¦‚é‚æ‚¤‚É‚È‚é
#extension GL_GOOGLE_include_directive:enable
//#include "XXX.glsl"

layout (early_fragment_tests) in;

layout (location = 0) in vec4 InColor;

layout (location = 0) out vec4 OutColor;

void main()
{
	OutColor = InColor;
}