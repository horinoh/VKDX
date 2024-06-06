#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec2 OutTexcoord;

void main()
{
	OutTexcoord = vec2(gl_VertexIndex / 2, gl_VertexIndex % 2);
	gl_Position = vec4(OutTexcoord.x * 2.0f - 1.0f, -(OutTexcoord.y * 2.0f - 1.0f), 0.0f, 1.0f);
}
