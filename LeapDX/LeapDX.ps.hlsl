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

//!< �~�͈͓̔��Ȃ�1.0f�A�͈͊O�Ȃ�0.0f��Ԃ� (Is inside circle return 1.0f, otherwise 0.0f)
float IsInCircle(const float2 pos, const float r)
{
    return max(sign(r * r - dot(pos, pos)), 0.0f);
}

float4 main(IN In) : SV_TARGET
{
#if 0
    //!< �C���� Leap �C���[�W�̕`�� (Draw rectified leap image)
    const float ArrayIndex = 1.0f - step(In.Texcoord.x, 0.5f);
    const float2 UV = float2(frac(In.Texcoord.x * 2.0f), In.Texcoord.y);
    
    const float2 DistortionIndex = DistortionMap.Sample(Sampler, float3(UV, ArrayIndex)).xy;
    clip(DistortionIndex);
    clip(1.0f - DistortionIndex);
    return float4(LeapMap.Sample(Sampler, float3(DistortionIndex, ArrayIndex)).rrr, 1.0f);
#elif 0
    //!< �f�B�X�g�[�V�����}�b�v�̕`�� (Draw distortion map)
    const float ArrayIndex = 1.0f - step(In.Texcoord.x, 0.5f);
    const float2 UV = float2(frac(In.Texcoord.x * 2.0f), In.Texcoord.y);
    
    //return LeapMap.Sample(Sampler, float3(UV, ArrayIndex));
    return DistortionMap.Sample(Sampler, float3(UV, ArrayIndex));
#else
    //!< �n���h�g���b�L���O�̕`�� (Draw hand tracking)
    static const float4 BGColor = { 0.0f, 0.0f, 0.0f, 1.0f }; //!< �F�����Z����̂ō��ł��邱�� (Must be black)
    static const float4 Colors[] = { { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } }; //!< �E�A����F (Right, left hand color)
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
