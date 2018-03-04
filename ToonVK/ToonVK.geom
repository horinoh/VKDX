#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InNormal[];

layout (set = 0, binding = 0) uniform Transform { mat4 Projection; mat4 View; mat4 World; };

layout (location = 0) out vec3 OutNormal;
layout (location = 1) out vec3 OutViewDirection;

layout (triangles, invocations = 1) in;
layout (triangle_strip, max_vertices = 3) out;
void main()
{
	const vec3 CamPos = vec3(View[3][0], View[3][1], View[3][2]);
	const mat4 PVW = Projection * View * World;

	for(int i=0;i<gl_in.length();++i) {
		gl_Position = PVW * gl_in[i].gl_Position;
		OutNormal = mat3(World) * InNormal[i];
		OutViewDirection = CamPos - (World * gl_Position).xyz;
		EmitVertex();
	}
	EndPrimitive();	
}
