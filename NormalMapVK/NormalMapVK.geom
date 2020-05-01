#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InNormal[];
layout (location = 1) in vec3 InTangent[];
layout (location = 2) in vec2 InTexcoord[];

layout (set = 0, binding = 0) uniform Transform { mat4 Projection; mat4 View; mat4 World; vec3 LocalCameraPosition; vec3 LocalLightDirection; };

layout (location = 0) out vec2 OutTexcoord;
layout (location = 1) out vec3 OutViewDirection;
layout (location = 2) out vec3 OutLightDirection;

layout (triangles, invocations = 1) in;
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
		const vec3 Binormal = cross(InNormal[i], InTangent[i]);
		gl_Position = PVW * gl_in[i].gl_Position;
		OutTexcoord = (TexTransform * vec4(InTexcoord[i], 0.0f, 1.0f)).xy;
		OutViewDirection = CamPos - (World * gl_Position).xyz;
		const vec3 V = LocalCameraPosition - gl_in[i].gl_Position.xyz;
		OutViewDirection = vec3(dot(V, InTangent[i]), dot(V, Binormal), dot(V, InNormal[i]));
		OutLightDirection = vec3(dot(LocalLightDirection, InTangent[i]), dot(LocalLightDirection, Binormal), dot(LocalLightDirection, InNormal[i]));
		EmitVertex();
	}
	EndPrimitive();	
}
