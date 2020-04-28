#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InNormal[];
layout (location = 1) in vec3 InTangent[];
layout (location = 2) in vec2 InTexcoord[];

layout (set = 0, binding = 0) uniform Transform { mat4 Projection; mat4 View; mat4 World; };

layout (location = 0) out vec3 OutNormal;
layout (location = 1) out vec4 OutTangent;
layout (location = 2) out vec2 OutTexcoord;
layout (location = 3) out vec3 OutViewDirection;

layout (triangles, invocations = 1) in;
//layout (line_strip, max_vertices = 3) out;
layout (triangle_strip, max_vertices = 3) out;
void main()
{
	const vec3 CamPos = -vec3(View[3][0], View[3][1], View[3][2]);
	const mat4 PVW = Projection * View * World;
	const mat4 TexTransform = mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	for(int i=0;i<gl_in.length();++i) {
		gl_Position = PVW * gl_in[i].gl_Position;
		OutNormal = mat3(World) * InNormal[i];
		OutTangent = vec4(mat3(World) * InTangent[i], 1.0f);
		OutTexcoord = (TexTransform * vec4(InTexcoord[i], 0.0f, 1.0f)).xy;
		OutViewDirection = CamPos - (World * gl_Position).xyz;
		EmitVertex();
	}
	EndPrimitive();	
//
//
//	for(int i=0;i<gl_in.length();++i) {
//		gl_Position = gl_in[i].gl_Position;
//		OutTexcoord = InTexcoord[i];
//		EmitVertex();
//	}
//	EndPrimitive();	
}
