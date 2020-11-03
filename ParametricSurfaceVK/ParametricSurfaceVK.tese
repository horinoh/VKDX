#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

const float PI = 4.0f * atan(1.0f);
vec2 GetUV_Torus(const vec2 uv) 
{
	//!< [0, 2PI] [0, 2PI]
	return fract(uv) * 2.0f * PI;
}
vec3 GetPosition_Torus(const vec2 uv)
{
	const vec2 UV = GetUV_Torus(uv);
	const vec2 R = vec2(0.5f, 1.0f);
	return vec3((R.y + R.x * cos(UV.y)) * cos(UV.x), (R.y + R.x * cos(UV.y)) * sin(UV.x), R.x * sin(UV.y));
}
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
vec2 GetUV_Snail(const vec2 uv) 
{
	return (fract(uv) * vec2(1.0f, -1.0f) + vec2(0.0f, 0.5f)) * 2.0f * PI;
}
vec3 GetPosition_Snail(const vec2 uv)
{
	const vec2 UV = GetUV_Snail(uv);
	return UV.xxx * vec3(cos(UV.y) * sin(UV.x), cos(UV.x) * cos(UV.y), -sin(UV.y));
}
vec2 GetUV_Pillow(const vec2 uv) 
{
	return (fract(uv) * vec2(1.0f, -1.0f) + vec2(0.0f, 1.0f)) * 2.0f * PI;
}
vec3 GetPosition_Pillow(const vec2 uv)
{
	const vec2 UV = GetUV_Pillow(uv);
	return vec3(cos(UV.x), cos(UV.y), 0.5f * sin(UV.x) * sin(UV.y));
}

layout (location = 0) out vec2 OutTexcoord;

layout (quads, equal_spacing, cw) in;
void main()
{
	gl_Position = vec4(GetPosition_Torus(gl_TessCoord.xy) * 0.5f, 1.0f);
	//gl_Position = vec4(GetPosition_Sphere(gl_TessCoord.xy) * 0.5f, 1.0f);
	//gl_Position = vec4(GetPosition_Snail(gl_TessCoord.xy) * 0.1f, 1.0f);
	//gl_Position = vec4(GetPosition_Pillow(gl_TessCoord.xy) * 0.5f, 1.0f);
	//gl_Position = vec4(2.0f * gl_TessCoord.xy - 1.0f, gl_TessCoord.z, 1.0f);

	OutTexcoord = vec2(gl_TessCoord.x, 1.0f - gl_TessCoord.y);
}

