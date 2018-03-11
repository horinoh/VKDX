#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 InNormal[];

layout (set = 0, binding = 0) uniform Transform { mat4 Projection; mat4 View; mat4 World; };

layout (location = 0) out vec3 OutNormal;
layout (location = 1) out vec3 OutViewDirection;
#if 0
layout (location = 2) out noperspective vec3 OutTriDistance;
#endif

#if 0
vec3 getTriangleDistance(const vec4 pos0, const vec4 pos1, const vec4 pos2)
{
	const mat4 Viewport = mat4(vec4(1280, 0.0f, 0.0f, 0.0f),
						       vec4(0.0f,  640, 0.0f, 0.0f),
						       vec4(0.0f, 0.0f, 1.0f, 0.0f),
							   vec4(1280, 640, 0.0f, 1.0f));

	//!< スクリーンスペース位置
	const vec3 scrPos0 = (Viewport * (pos0 / pos0.w)).xyz;
	const vec3 scrPos1 = (Viewport * (pos1 / pos1.w)).xyz;
	const vec3 scrPos2 = (Viewport * (pos2 / pos2.w)).xyz;
	//!< なす角
	const float a = length(scrPos1 - scrPos2);
	const float b = length(scrPos2 - scrPos0);
	const float c = length(scrPos1 - scrPos0);
	const float alpha = acos((b * b + c * c - a * a) / (2.0f * b * c));
	const float beta  = acos((a * a + c * c - b * b) / (2.0f * a * c));
	//!< 垂線
	const float ha = abs(c * sin(beta));
	const float hb = abs(c * sin(alpha));
	const float hc = abs(b * sin(alpha));
	return vec3(ha, hb, hc);
}
#endif

layout (triangles, invocations = 1) in;
layout (triangle_strip, max_vertices = 3) out;
void main()
{
	const vec3 CamPos = vec3(View[3][0], View[3][1], View[3][2]);
	const mat4 PVW = Projection * View * World;

#if 0
	const vec4 pos[3] = { PVW * gl_in[0].gl_Position, PVW * gl_in[1].gl_Position, PVW * gl_in[2].gl_Position };
	const vec3 d = getTriangleDistance(pos[0], pos[1], pos[2]);

	gl_Position = pos[0];
	OutNormal = mat3(World) * InNormal[0];
	OutViewDirection = CamPos - (World * gl_Position).xyz;
	OutTriDistance = vec3(d.x, 0.0f, 0.0f);
	EmitVertex();

	gl_Position = pos[1];
	OutNormal = mat3(World) * InNormal[1];
	OutViewDirection = CamPos - (World * gl_Position).xyz;
	OutTriDistance = vec3(0.0f, d.y, 0.0f);
	EmitVertex();

	gl_Position = pos[2];
	OutNormal = mat3(World) * InNormal[2];
	OutViewDirection = CamPos - (World * gl_Position).xyz;
	OutTriDistance = vec3(0.0f, 0.0f, d.z);
	EmitVertex();
#else
	for(int i=0;i<gl_in.length();++i) {
		gl_Position = PVW * gl_in[i].gl_Position;
		OutNormal = mat3(World) * InNormal[i];
		OutViewDirection = CamPos - (World * gl_Position).xyz;
		EmitVertex();
	}
#endif
	EndPrimitive();	
}
