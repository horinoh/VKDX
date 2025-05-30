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

layout (location = 0) out vec3 OutNormal;

//!< VkPipelineRasterizationStateCreateInfo.frontFace == VK_FRONT_FACE_COUNTER_CLOCKWISE とかち合うのでここでは cw を指定
layout (quads, equal_spacing, cw) in;
void main()
{
	gl_Position = vec4(GetPosition_Torus(gl_TessCoord.xy) * 0.5f, 1.0f);
	OutNormal = GetNormal_Torus(gl_TessCoord.xy, gl_Position.xyz);
}

