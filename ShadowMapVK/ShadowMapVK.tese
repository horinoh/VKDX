#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

const float PI = 4.0f * atan(1.0f);
vec2 GetUV_Torus(const vec2 uv) 
{
	return fract(uv) * 2.0f * PI;
}
vec3 GetPosition_Torus(const vec2 uv)
{
	const vec2 UV = GetUV_Torus(uv);
	const vec2 R = vec2(0.5f, 1.0f);
	return vec3((R.y + R.x * cos(UV.y)) * cos(UV.x), (R.y + R.x * cos(UV.y)) * sin(UV.x), R.x * sin(UV.y));
}
vec3 GetNormal_Torus(const vec2 uv, const vec3 pos)
{
	const vec2 du = vec2(0.01f, 0.0f);
	const vec2 dv = vec2(0.0f, 0.01f);
	return normalize(cross(GetPosition_Torus(uv + du) - pos, GetPosition_Torus(uv + dv) - pos));
}

vec2 GetUV_Plane(const vec2 uv) { return uv * 2.0f - 1.0f; }
vec3 GetPosition_PlaneXZ(const vec2 uv)
{
	const vec2 UV = GetUV_Plane(uv);
	return vec3(UV.y, 0.0f, UV.x);
}
vec3 GetNormal_PlaneXZ(const vec2 uv, const vec3 pos) { return vec3(0.0f, 1.0f, 0.0f); }

layout (location = 0) in int [] InInstanceIndex;

layout (location = 0) out vec3 OutNormal;
layout (location = 1) out vec2 OutTexcoord;

layout (quads, equal_spacing, cw) in;
void main()
{
	const vec2 Mask[] = { vec2(1.0f, 0.0f), vec2(0.0f, 1.0f) };
	gl_Position = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	OutNormal = vec3(0.0f, 0.0f, 0.0f);

	gl_Position.xyz += GetPosition_Torus(gl_TessCoord.xy) * 0.5f * Mask[InInstanceIndex[gl_PrimitiveID]].xxx;
	OutNormal += GetNormal_Torus(gl_TessCoord.xy, gl_Position.xyz) * Mask[InInstanceIndex[gl_PrimitiveID]].xxx;

	gl_Position.xyz += (GetPosition_PlaneXZ(gl_TessCoord.xy) - vec3(0.0f, 0.75f, 0.0f)) * Mask[InInstanceIndex[gl_PrimitiveID]].yyy;
	OutNormal += GetNormal_PlaneXZ(gl_TessCoord.xy, gl_Position.xyz) * Mask[InInstanceIndex[gl_PrimitiveID]].yyy;

	OutTexcoord = vec2(gl_TessCoord.x, 1.0f - gl_TessCoord.y);
}

