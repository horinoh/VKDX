#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

const float PI = 4.0f * atan(1.0f);
vec2 GetUV_Sphere(const vec2 uv) 
{
	return (fract(uv) * vec2(1.0f, -1.0f) + vec2(0.0f, 0.5f)) * 2.0f * PI;
}
vec3 GetPosition_Sphere(const vec2 uv)
{
	const vec2 UV = GetUV_Sphere(uv);
	const vec3 R = vec3(1.0f, 1.0f, 1.0f);
	return R * vec3(cos(UV.x) * sin(UV.y), sin(UV.x) * sin(UV.y), cos(UV.y));
}

layout (quads, equal_spacing, ccw) in;
void main()
{
	gl_Position = vec4(GetPosition_Sphere(gl_TessCoord.xy) * 0.5f, 1.0f);
}

