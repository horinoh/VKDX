#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

const float PI = 4.0f * atan(1.0f);

vec2 GetUV_Sphere(const vec2 uv) 
{
	//!< [0, 2PI] [PI, -PI]
	return (fract(uv) * vec2(1.0f, -1.0f) + vec2(0.0f, 0.5f)) * 2.0f * PI;
}
vec3 GetPosition_Sphere(const vec2 uv)
{
	const vec2 UV = GetUV_Sphere(uv);
	const vec3 R = vec3(1.0f, 1.0f, 1.0f);
	return R * vec3(cos(UV.x) * sin(UV.y), sin(UV.x) * sin(UV.y), cos(UV.y));
}
vec3 GetNormal_Sphere(const vec2 uv, const vec3 pos)
{
	const vec2 du = vec2(0.01f, 0.0f);
	const vec2 dv = vec2(0.0f, 0.01f);
	return normalize(cross(GetPosition_Sphere(uv + du) - pos, GetPosition_Sphere(uv + dv) - pos));
}

layout (location = 0) in int InInstanceIndex[];

layout (location = 0) out vec3 OutNormal;
layout (location = 1) out vec2 OutTexcoord;
layout (location = 2) out int OutInstanceIndex[];

layout (quads, equal_spacing, cw) in;
void main()
{
	//gl_Position = vec4(GetPosition_Sphere(gl_TessCoord.xy) * 0.5f, 1.0f);
	gl_Position = vec4(GetPosition_Sphere(vec2(gl_TessCoord.x, 1 - gl_TessCoord.y)) * 0.5f, 1.0f);
	OutNormal = GetNormal_Sphere(gl_TessCoord.xy, gl_Position.xyz);
	OutTexcoord = vec2(gl_TessCoord.x, 1.0f - gl_TessCoord.y);
	OutInstanceIndex[0] = InInstanceIndex[0];
}

