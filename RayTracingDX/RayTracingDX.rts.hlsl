//!< DX�ł̓��C�g���[�V���O�n�̊e��V�F�[�_��1�t�@�C���ɂ܂Ƃ߂邵���Ȃ��H

//struct RayDesc
//{
//    float3 Origin;
//    float TMin;
//    float3 Direction;
//    float TMax;
//};

struct Payload
{
    float4 Color;
};

RaytracingAccelerationStructure Scene : register(t0);
ByteAddressBuffer Vertices : register(t1);
ByteAddressBuffer Indices : register(t2);
RWTexture2D<float4> RenderTarget : register(u0);
//ConstantBuffer<XXX> XXX : register(b0);

[shader("raygeneration")]
void OnRayGeneration()
{
    const float2 t = (float2)DispatchRaysIndex().xy / DispatchRaysDimensions().xy;
    
    //!< �y�C���[�h�������� (Initialize payload)
    Payload Pay;
    Pay.Color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    //!< ���C
    RayDesc Ray;
    Ray.TMin = 0.0f;
    Ray.TMax = 10000.0f;
    //Ray.Origin = mul(InvView, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
    //const float4 Screen = mul(InvViewProj, float4(-2.0f * t + 1.0f, 0.0f, 1.0f));
    //Ray.Direction = normalize(Screen.xyz / Screen.w - Ray.Origin.xyz);
    
    //!< �g���[�V���O
    TraceRay(Scene, RAY_FLAG_NONE /*RAY_FLAG_CULL_BACK_FACING_TRIANGLES*/, ~0, 0, 1, 0, Ray, Pay);
    
    //!< ���ʂ������_�[�^�[�Q�b�g��
    RenderTarget[DispatchRaysIndex().xy] = Pay.Color;
}
[shader("closesthit")]
void OnClosestHit(inout Payload Pay, in BuiltInTriangleIntersectionAttributes BITIA)
{
    //!< a0, a1, a2 �̂R���_�̏ꍇ�Abarycentris.xy �͂��ꂼ�� a1, a2 �̃E�G�C�g
    //!< a = a0 + barycentrics.x * (a1 - a0) + barycentrics.y * (a2 - a0)
    const float3 BaryCentrics = float3(1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y, BITIA.barycentrics.x, BITIA.barycentrics.y);
    
    Pay.Color.rgb = float3(0.0f, 1.0f, 0.0f);
}
[shader("miss")]
void OnMiss(inout Payload Pay)
{
    Pay.Color.rgb = float3(0.529411793f, 0.807843208f, 0.921568692f);
}

//!< �����ł͎g�p���Ȃ� (Not used here)
//[shader("intersection")]
//[shader("anyhit")]
//[shader("callable")]