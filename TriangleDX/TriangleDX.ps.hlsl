struct IN
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

#if 1
//!< HDR使用(USE_HDR)かつ、スワップチェインのカラーフォーマット(DXGI_FORMAT_R10G10B10A2_UNORM) を使用した場合に必要になるガンマ補正
float3 Rec709To2020(const float3 In) 
{
	static const float3x3 m = {
		{ 0.627404f, 0.329282f, 0.0433136f },
		{ 0.069097f, 0.91954f, 0.0113612f },
		{ 0.0163916f, 0.0880132f, 0.895595f },
	};
	return mul(m, In);
}
float3 LinearToST2084(const float3 In)
{
	float3 t = pow(abs(In), 0.1593017578f);
	return pow((0.8359375f + 18.8515625f * t) / (1.0f + 18.6875f * t), 78.84375f);
}
float3 ToHDR10(const float3 In)
{
	return LinearToST2084(Rec709To2020(In) * (80.0f / 10000.0f));
}
#endif

float4 main(IN In) : SV_TARGET //!< ここでの出力はラスタライザが使用する情報(システム情報)なので"SV_"を付ける
{
	return In.Color;
	//return float4(ToHDR10(In.Color.rgb).rgb, In.Color.a);
}
