#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InNormal[];
layout (location = 1) in int InInstanceIndex[];

#define INSTANCE_COUNT 10
layout (set = 0, binding = 0) uniform Transform {
	mat4 Projection;
	mat4 View; 
	mat4 World[INSTANCE_COUNT]; 
};

layout (location = 0) out vec3 OutNormal;

layout (triangles, invocations = 1) in;
layout (triangle_strip, max_vertices = 3) out;
void main()
{
	const mat4 W = World[InInstanceIndex[0]];
	const mat4 PVW = Projection * View * W;

	for(int i = 0; i < gl_in.length();++i) {
		gl_Position = PVW * gl_in[i].gl_Position;
		OutNormal = mat3(W) * InNormal[i];
		EmitVertex();
	}
	EndPrimitive();	
}
