#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out int OutInstanceIndex;

void main()
{
	OutInstanceIndex = gl_InstanceIndex;
}
