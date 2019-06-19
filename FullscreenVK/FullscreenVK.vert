#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec2 OutTexcoord;

void main()
{
	const vec2 Position[] = vec2[](
		vec2(-1.0f, 1.0f),  //!< LT
		vec2(-1.0f, -1.0f), //!< LB
		vec2(1.0f, 1.0f), //!< RT
		vec2(1.0f, -1.0f) //!< RB
	);
	
	gl_Position = vec4(Position[gl_VertexIndex], 0.0f, 1.0f);
	OutTexcoord = vec2(gl_Position.x, -gl_Position.y) * 0.5f + 0.5f;
}
