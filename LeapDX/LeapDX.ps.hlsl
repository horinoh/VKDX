struct IN
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
};

Texture2DArray<float4> LeapMap : register(t0, space0);
Texture2DArray<float4> DistortionMap : register(t1, space0);
SamplerState Sampler : register(s0, space0);
struct Tracking
{
    float4 Hands[2][5][4];
};
ConstantBuffer<Tracking> Tr : register(b0, space0);

//!< 円の範囲内なら1.0f、範囲外なら0.0fを返す (Is inside circle return 1.0f, otherwise 0.0f)
float IsInCircle(const float2 pos, const float r)
{
    return max(sign(r * r - dot(pos, pos)), 0.0f);
}

float4 main(IN In) : SV_TARGET
{
#if 0
    //!< 修正済 Leap イメージの描画 (Draw rectified leap image)
    const float ArrayIndex = 1.0f - step(In.Texcoord.x, 0.5f);
    const float2 UV = float2(frac(In.Texcoord.x * 2.0f), In.Texcoord.y);
    
    const float2 DistortionIndex = DistortionMap.Sample(Sampler, float3(UV, ArrayIndex)).xy;
    clip(DistortionIndex);
    clip(1.0f - DistortionIndex);
    return float4(LeapMap.Sample(Sampler, float3(DistortionIndex, ArrayIndex)).rrr, 1.0f);
#elif 0
    //!< ディストーションマップの描画 (Draw distortion map)
    const float ArrayIndex = 1.0f - step(In.Texcoord.x, 0.5f);
    const float2 UV = float2(frac(In.Texcoord.x * 2.0f), In.Texcoord.y);
    
    //return LeapMap.Sample(Sampler, float3(UV, ArrayIndex));
    return DistortionMap.Sample(Sampler, float3(UV, ArrayIndex));
#else
    //!< ハンドトラッキングの描画 (Draw hand tracking)
    static const float4 BGColor = { 0.0f, 0.0f, 0.0f, 1.0f }; //!< 色を加算するので黒であること (Must be black)
    static const float4 Colors[] = { { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }; //!< 右、左手色 (Right, left hand color)
    float4 OutColor = BGColor;
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                OutColor.rgb += lerp(BGColor, Colors[i], IsInCircle(Tr.Hands[i][j][k].xz - (In.Texcoord - 0.5f) * 2.0f, Tr.Hands[i][j][k].y * 0.1f)).rgb;
            }
        }
    }
    return OutColor;
#endif
}
