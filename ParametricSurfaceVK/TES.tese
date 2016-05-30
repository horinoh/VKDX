#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

out gl_PerVertex
{
	vec4 gl_Position;
};

layout (quads, equal_spacing, ccw) in;
void main()
{
//	gl_Position = vec4(GetPosition_Torus(gl_TessCoord.xy), 1.0f);
}

