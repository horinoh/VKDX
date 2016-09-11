#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (triangles, invocations = 1) in;
layout (triangle_strip, max_vertices = 3) out;
void main()
{
	//const mat4 PVW = Projection * View * World;

	for(int i=0;i<gl_in.length();++i) {
		//gl_Position = PVW * gl_in[i].gl_Position;
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();	
}
