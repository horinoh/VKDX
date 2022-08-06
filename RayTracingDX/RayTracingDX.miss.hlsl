struct Payload
{
    float3 Color;
};

SamplerState Sampler : register(s0, space0);
TextureCube CubeMap : register(t1, space0);

[shader("miss")]
void OnMiss(inout Payload Pay)
{
    //!< �����I�Ƀ~�b�v�}�b�v������ł��Ȃ��̂� SampleLevel() �Ŗ����I�Ƀ~�b�v���x�����w�肷��
    Pay.Color = CubeMap.SampleLevel(Sampler, WorldRayDirection(), 0.0f).rgb;
}
