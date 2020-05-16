#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InNormal[];
layout (location = 1) in vec2 InTexcoord[];

layout (location = 0) out vec3 OutNormal;
layout (location = 1) out vec2 OutTexcoord;
layout (location = 2) out vec2 OutDepth;

layout (triangles, invocations = 1) in;
layout (triangle_strip, max_vertices = 3) out;
void main()
{
	for(int i=0;i<gl_in.length();++i) {
		//gl_Position = PVW * gl_in[i].gl_Position;
		//OutNormal = mat3(World) * InNormal[i];
		gl_Position = gl_in[i].gl_Position;
		OutNormal = InNormal[i];
		OutTexcoord = InTexcoord[i];
		OutDepth = gl_Position.zw;
		EmitVertex();
	}
	EndPrimitive();	
}
