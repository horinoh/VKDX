struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

SamplerState Sampler : register(s0, space0);
Texture2D Texture : register(t0, space0);

struct HOLO_DRAW
{
	float Pitch;
	float Tilt;
	float Center;
	int InvView;
	float Subp;
	float DisplayAspect;
	int Ri, Bi;

	float3 Tile;
	//float2 ViewPortion;
	//float QuiltAspect;
	//int Overscan;
	//int QuiltInvert;
};
ConstantBuffer<HOLO_DRAW> HoloDraw : register(b0, space0);

static const float2 ViewPortion = float2(1.0f, 1.0f);
static const float QuiltAspect = HoloDraw.DisplayAspect;
static const int Overscan = 0;
static const int QuiltInvert = 0;

float2 TexArr(const float3 UVZ) {
	const float z = floor(UVZ.z * HoloDraw.Tile.z);
	const float x = (fmod(z, HoloDraw.Tile.x) + UVZ.x) / HoloDraw.Tile.x;
	const float y = (floor(z / HoloDraw.Tile.x) + UVZ.y) / HoloDraw.Tile.y;
	return float2(x, y) * ViewPortion.xy;
}

float4 main(IN In) : SV_TARGET
{
#if 0
	return Texture.Sample(Sampler, In.Texcoord);
#else
	//float Invert = 1.0f;
	//if (1 == InvView + QuiltInvert) { Invert = -1.0f; }

	const float modx = saturate(
		step(QuiltAspect, HoloDraw.DisplayAspect) * step((float)Overscan, 0.5f) +
		step(HoloDraw.DisplayAspect, QuiltAspect) * step(0.5f, (float)Overscan)
	);

	float3 nuv = float3(In.Texcoord, 0.0f);
	nuv -= 0.5f;
	{
		nuv.x = modx * nuv.x * HoloDraw.DisplayAspect / QuiltAspect + (1.0f - modx) * nuv.x;
		nuv.y = modx * nuv.y + (1.0f - modx) * nuv.y * QuiltAspect / HoloDraw.DisplayAspect;
	}
	nuv += 0.5f;

	clip(nuv);
	clip(1.0f - nuv);

	float4 rgb[3];
	for (int i = 0; i < 3; ++i) {
		nuv.z = (In.Texcoord.x + i * HoloDraw.Subp + In.Texcoord.y * HoloDraw.Tilt) * HoloDraw.Pitch - HoloDraw.Center;
		nuv.z = fmod(nuv.z + ceil(abs(nuv.z)), 1.0f);
		rgb[i] = Texture.Sample(Sampler, TexArr(nuv));
	}
	return float4(rgb[HoloDraw.Ri].r, rgb[1].g, rgb[HoloDraw.Bi].b, 1.0f);
#endif
}
