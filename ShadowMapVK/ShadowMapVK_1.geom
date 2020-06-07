#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 1) uniform Transform { mat4 Projection; mat4 View; mat4 World; mat4 LightProjection; mat4 LightView; };

layout (location = 0) out vec4 OutTexcoord;

layout (triangles, invocations = 1) in;
layout (triangle_strip, max_vertices = 3) out;
void main()
{
	const mat4 PVW = Projection * View * World;
	const mat4 LPVW = LightProjection * LightView * World; 

	for(int i=0;i<gl_in.length();++i) {
		gl_Position = PVW * gl_in[i].gl_Position;
		OutTexcoord = LPVW * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();	
}
