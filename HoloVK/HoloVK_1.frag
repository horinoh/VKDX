#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (early_fragment_tests) in;

layout (location = 0) in vec2 InTexcoord;

layout (location = 0) out vec4 OutColor;

layout (set=0, binding=0) uniform sampler2D Sampler2D;

layout (push_constant) uniform HOLO_DRAW
{
	float Pitch;
	float Tilt;
	float Center;
	int InvView;
	float Subp;
	float DisplayAspect;
	int Ri, Bi;

	vec3 Tile;
//	vec2 ViewPortion;
//	float QuiltAspect;
//	int Overscan;
//	int QuiltInvert;
} HoloDraw;

const float Pitch = 246.866f;
const float Tilt = -0.185377f;
const float Center = 0.565845f;
const int InvView = 1;
const float Subp = 0.000217014f;
const float DisplayAspect = 0.75f;
const int Ri = 0, Bi = 2;

const vec3 Tile = vec3(8.0f, 6.0f, 48.0f);
const vec2 ViewPortion = vec2(1.0f, 1.0f);//vec2(1536.0f * 8.0f / 3360.0f, 2048.0f * 6.0f / 3360.0f);
const float QuiltAspect = DisplayAspect;
const int Overscan = 0;
const int QuiltInvert = 0;

vec2 TexArr(const vec3 UVZ) {
	const float z = floor(UVZ.z * Tile.z);
	const float x = (mod(z, Tile.x) + UVZ.x) / Tile.x;
	const float y = (floor(z / Tile.x) + UVZ.y) / Tile.y;
	return vec2(x, y) * ViewPortion.xy;
}

void main()
{
	//float Invert = 1.0f;
	//if (1 == InvView + QuiltInvert) { Invert = -1.0f; }

	const float modx = clamp(
		step(QuiltAspect, DisplayAspect) * step(float(Overscan), 0.5f) +
		step(DisplayAspect, QuiltAspect) * step(0.5f, float(Overscan))
	, 0.0f, 1.0f);

	vec3 nuv = vec3(InTexcoord, 0.0f);
	nuv -= 0.5f;
	{
		nuv.x = modx * nuv.x * DisplayAspect / QuiltAspect + (1.0f - modx) * nuv.x;
		nuv.y = modx * nuv.y + (1.0f - modx) * nuv.y * QuiltAspect / DisplayAspect;
	}
	nuv += 0.5f;

	 if (any(lessThan(nuv, vec3(0.0f, 0.0f, 0.0f)))) discard;
	 if (any(lessThan(1.0f - nuv, vec3(0.0f, 0.0f, 0.0f)))) discard;

	vec4 rgb[3];
	for (int i = 0; i < 3; ++i) {
		nuv.z = (InTexcoord.x + i * Subp + InTexcoord.y * Tilt) * Pitch - Center;
		nuv.z = mod(nuv.z + ceil(abs(nuv.z)), 1.0f);
		rgb[i] = texture(Sampler2D, TexArr(nuv));
	}
	OutColor = vec4(rgb[Ri].r, rgb[1].g, rgb[Bi].b, 1.0f);
}