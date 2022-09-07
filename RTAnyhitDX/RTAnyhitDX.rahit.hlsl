struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

//SamplerState Sampler : register(s0, space0);
//Texture2D TransparentMap : register(t1, space0);

//!< Anyhit �V�F�[�_���Ăяo�����ɂ͑Ώۂ̃I�u�W�F�N�g��������(��D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE)�ł���K�v������̂Œ���
[shader("anyhit")]
void OnAnyHit(inout PAYLOAD Payload, in BuiltInTriangleIntersectionAttributes BITIA)
{
    //const float2 UV = ((float2) DispatchRaysIndex().xy + 0.5f) / DispatchRaysDimensions().xy;
    //if (TransparentMap.SampleLevel(Sampler, UV, 0.0f).a < 0.5f) {
    //    IgnoreHit(); //!< ������������p��A�ĊJ
    //    AcceptHitAndEndSearch(); //!< ����ȏ�̌�������͕s�K�v
    //}
    
    //!< �s���͗l
    const float2 pos = float2(DispatchRaysIndex().xy / 8);
    if (fmod(pos.x + fmod(pos.y, 2.0f), 2.0f) < 0.5f) {
        IgnoreHit(); //!< ������������p��A�ĊJ
        //AcceptHitAndEndSearch(); //!< ����ȏ�̌�������͕s�K�v
    }
}
