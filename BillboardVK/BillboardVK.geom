#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) out vec2 Texcoord;

//layout (set=0, binding=0) uniform Transform { mat4 Projection; mat4 View; mat4 World; }
//layout (push_constants) uniform Transform { mat4 Projection; mat4 View; mat4 World; } PushConstant;

layout (triangles, invocations = 1) in;
layout (triangle_strip, max_vertices = 4) out;
void main()
{
	//!< #VK_TODO
	const mat4 World = mat4(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
	const vec3 CameraPosition = vec3(0, 0, -1);
	const vec3 Axis = vec3(0, 1, 0);
	const mat4 PVW = mat4(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);

	//const mat4 PVW = Projection * View * World;
	//const mat4 PVW = PushConstant.Projection * PushConstant.View * PushConstant.World;

	const vec3 Center = (gl_in[0].gl_Position.xyz + gl_in[1].gl_Position.xyz + gl_in[2].gl_Position.xyz) / 3.0f;
	const vec3 Forward = normalize(CameraPosition - (World * vec4(Center, 1.0f)).xyz);
	const vec3 Right = cross(Forward, Axis);
	const float Scale = 0.05f;

	gl_Position = PVW * vec4(Center - Scale * Right - Scale * Axis, 1.0f); //!< RB
	Texcoord = vec2(1.0f, 1.0f);
	EmitVertex();

	gl_Position = PVW * vec4(Center - Scale * Right + Scale * Axis, 1.0f); //!< RU
	Texcoord = vec2(1.0f, 0.0f);
	EmitVertex();

	gl_Position = PVW * vec4(Center + Scale * Right - Scale * Axis, 1.0f); //!< LB
	Texcoord = vec2(0.0f, 1.0f);
	EmitVertex();

	gl_Position = PVW * vec4(Center + Scale * Right + Scale * Axis, 1.0f); //!< LU
	Texcoord = vec2(0.0f, 0.0f);
	EmitVertex();

	EndPrimitive();
}
