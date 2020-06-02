struct IN
{
	float3 Position : POSITION;
	float4 Color : COLOR;
};

struct OUT
{
	float4 Position : SV_POSITION; //!< ここでの出力はラスタライザが使用する情報(システム情報)なので"SV_"を付ける
	float4 Color : COLOR;
}; 

OUT main(IN In)
{
	OUT Out;

	Out.Position = float4(In.Position, 1.0f);
	Out.Color = In.Color;

#if 0
	//!< ピクセル指定
	const float2 Inv = float2(2.0f / 1280.0f, 2.0f / 720.0f);
	const float4x4 Transform = float4x4(Inv.x, 0.0f, 0.0f, -1.0f,
		0.0f, -Inv.y, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0, 0.0f, 0.0f, 1.0f);
	Out.Position = mul(Transform, Out.Position);
#endif

	return Out;
}
