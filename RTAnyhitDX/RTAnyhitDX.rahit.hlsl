struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

//SamplerState Sampler : register(s0, space0);
//Texture2D TransparentMap : register(t1, space0);

//!< Anyhit シェーダが呼び出されるには対象のオブジェクトが半透明(非D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE)である必要があるので注意
[shader("anyhit")]
void OnAnyHit(inout PAYLOAD Payload, in BuiltInTriangleIntersectionAttributes BITIA)
{
    //const float2 UV = ((float2) DispatchRaysIndex().xy + 0.5f) / DispatchRaysDimensions().xy;
    //if (TransparentMap.SampleLevel(Sampler, UV, 0.0f).a < 0.5f) {
    //    IgnoreHit(); //!< 交差判定を棄却後、再開
    //    AcceptHitAndEndSearch(); //!< これ以上の交差判定は不必要
    //}
    
    //!< 市松模様
    const float2 pos = float2(DispatchRaysIndex().xy / 8);
    if (fmod(pos.x + fmod(pos.y, 2.0f), 2.0f) < 0.5f) {
        IgnoreHit(); //!< 交差判定を棄却後、再開
        //AcceptHitAndEndSearch(); //!< これ以上の交差判定は不必要
    }
}
