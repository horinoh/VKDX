#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (vertices = 4) out;
void main()
{
	const float t = 15;
	for(int i=0;i<gl_TessLevelOuter.length();++i) {
		gl_TessLevelOuter[i] = t;
	}
	for(int i=0;i<gl_TessLevelInner.length();++i) {
		gl_TessLevelInner[i] = t;
	}
	gl_out[ gl_InvocationID ].gl_Position = gl_in[ gl_InvocationID ].gl_Position;
}

