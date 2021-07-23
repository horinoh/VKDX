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

const vec2 ViewPortion = vec2(1.0f, 1.0f);
float QuiltAspect = HoloDraw.DisplayAspect;
const int Overscan = 0;
const int QuiltInvert = 0;

vec2 TexArr(const vec3 UVZ) {
	const float z = floor(UVZ.z * HoloDraw.Tile.z);
	const float x = (mod(z, HoloDraw.Tile.x) + UVZ.x) / HoloDraw.Tile.x;
	const float y = (floor(z / HoloDraw.Tile.x) + UVZ.y) / HoloDraw.Tile.y;
	return vec2(x, y) * ViewPortion.xy;
}

void main()
{
	//float Invert = 1.0f;
	//if (1 == InvView + QuiltInvert) { Invert = -1.0f; }

	const float modx = clamp(
		step(QuiltAspect, HoloDraw.DisplayAspect) * step(float(Overscan), 0.5f) +
		step(HoloDraw.DisplayAspect, QuiltAspect) * step(0.5f, float(Overscan))
	, 0.0f, 1.0f);

	vec3 nuv = vec3(InTexcoord, 0.0f);
	nuv -= 0.5f;
	{
		nuv.x = modx * nuv.x * HoloDraw.DisplayAspect / QuiltAspect + (1.0f - modx) * nuv.x;
		nuv.y = modx * nuv.y + (1.0f - modx) * nuv.y * QuiltAspect / HoloDraw.DisplayAspect;
	}
	nuv += 0.5f;

	 if (any(lessThan(nuv, vec3(0.0f, 0.0f, 0.0f)))) discard;
	 if (any(lessThan(1.0f - nuv, vec3(0.0f, 0.0f, 0.0f)))) discard;

	vec4 rgb[3];
	for (int i = 0; i < 3; ++i) {
		nuv.z = (InTexcoord.x + i * HoloDraw.Subp + InTexcoord.y * HoloDraw.Tilt) * HoloDraw.Pitch - HoloDraw.Center;
		nuv.z = mod(nuv.z + ceil(abs(nuv.z)), 1.0f);
		rgb[i] = texture(Sampler2D, TexArr(nuv));
	}
	OutColor = vec4(rgb[HoloDraw.Ri].r, rgb[1].g, rgb[HoloDraw.Bi].b, 1.0f);
}