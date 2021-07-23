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

static const float Pitch = 246.866f;
static const float Tilt = -0.185377f;
static const float Center = 0.565845f;
static const int InvView = 1;
static const float Subp = 0.000217014f;
static const float DisplayAspect = 0.75f;
static const int Ri = 0, Bi = 2;

static const float3 Tile = float3(8.0f, 6.0f, 48.0f);
static const float2 ViewPortion = float2(1.0f, 1.0f);//float2(1536.0f * 8.0f / 3360.0f, 2048.0f * 6.0f / 3360.0f);
static const float QuiltAspect = DisplayAspect;
static const int Overscan = 0;
static const int QuiltInvert = 0;

float2 TexArr(const float3 UVZ) {
	const float z = floor(UVZ.z * Tile.z);
	const float x = (fmod(z, Tile.x) + UVZ.x) / Tile.x;
	const float y = (floor(z / Tile.x) + UVZ.y) / Tile.y;
	return float2(x, y) * ViewPortion.xy;
}

float4 main(IN In) : SV_TARGET
{
	//float Invert = 1.0f;
	//if (1 == InvView + QuiltInvert) { Invert = -1.0f; }

	const float modx = saturate(
		step(QuiltAspect, DisplayAspect) * step((float)Overscan, 0.5f) +
		step(DisplayAspect, QuiltAspect) * step(0.5f, (float)Overscan)
	);

	float3 nuv = float3(In.Texcoord, 0.0f);
	nuv -= 0.5f;
	{
		nuv.x = modx * nuv.x * DisplayAspect / QuiltAspect + (1.0f - modx) * nuv.x;
		nuv.y = modx * nuv.y + (1.0f - modx) * nuv.y * QuiltAspect / DisplayAspect;
	}
	nuv += 0.5f;

	clip(nuv);
	clip(1.0f - nuv);

	float4 rgb[3];
	for (int i = 0; i < 3; ++i) {
		nuv.z = (In.Texcoord.x + i * Subp + In.Texcoord.y * Tilt) * Pitch - Center;
		nuv.z = fmod(nuv.z + ceil(abs(nuv.z)), 1.0f);
		rgb[i] = Texture.Sample(Sampler, TexArr(nuv));
	}
	return float4(rgb[Ri].r, rgb[1].g, rgb[Bi].b, 1.0f);
}
