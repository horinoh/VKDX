#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec2 OutTexcoord;

//!< [0]T(0,0)P(-1,1)	[2]T(1,0)P(1,1)
//!< [1]T(0,1)P(-1,-1)	[3]T(1,1)P(1,-1)
void main()
{
	OutTexcoord = vec2(gl_VertexIndex / 2, gl_VertexIndex % 2);
	gl_Position = vec4(OutTexcoord.x * 2.0f - 1.0f, -(OutTexcoord.y * 2.0f - 1.0f), 0.0f, 1.0f);

#if 0
	const mat4 TexTransform = mat4(4.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 4.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f);
	OutTexcoord = (TexTransform * vec4(OutTexcoord, 0.0f, 1.0f)).xy;
#endif
}
