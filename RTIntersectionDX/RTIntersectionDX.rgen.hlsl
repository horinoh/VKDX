//!< �X���b�h
//!<    uint3 DispatchRaysIndex(), DispatchRaysDimemsions()

//!< ���C
//!<    uint RayFlags()
//!<    float3 WorldRayOrigin(), WorldRayDirection(), ObjectRayOrigin(), ObjectRayDirection()
//!<    float RayTMin(), RayTCurrent()

//!< �C���f�b�N�X
//!<    uint InstanceID()(D3D12_RAYTRACING_INSTANCE_DESC.InstanceID), InstanceIndex(), GeometryIndex(), PrimitiveIndex()

//!< �I�u�W�F�N�g <-> ���[���h
//!<    float4x3 ObjectToWorld4x3()
//!<    float3x4 ObjectToWorld3x4()
//!<    float4x3 WorldToObject4x3()
//!<    float3x4 WorldToObject3x4()

//!< �q�b�g
//!<    uint HitKind()(HIT_KIND_TRIANGLE_FRONT_FACE, HIT_KIND_TRIANGLE_BACK_FACE)
    
struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

RaytracingAccelerationStructure TLAS : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0, space0);

[shader("raygeneration")]
void OnRayGeneration()
{
    //!< �y�C���[�h�������� (Initialize payload)
    PAYLOAD Payload;
    Payload.Color = float3(0.0f, 0.0f, 0.0f);
    Payload.Recursive = 0;
    
    //!< +0.5f �̓s�N�Z���̒��S�ɂ��邽��
    const float2 UV = ((float2)DispatchRaysIndex().xy + 0.5f) / DispatchRaysDimensions().xy  * 2.0f - 1.0f;
    const float2 UpY = float2(UV.x, -UV.y);

    //!< ���C (Ray)
    RayDesc Ray;
    Ray.TMin = 0.001f; //!< float �G���[�΍� �� 0.0f �̏����Ȓl�ɂ���
    Ray.TMax = 100000.0f;
    Ray.Origin = float3(UpY, -1.0f);
    Ray.Direction = float3(0.0f, 0.0f, 1.0f);

    //!< ���C�g���[�V���O (RayTracing)
    //!< �t���O
    //!<    RAY_FLAG_NONE,
    //!<    RAY_FLAG_FORCE_OPAQUE, RAY_FLAG_FORCE_NON_OPAQUE,
    //!<    RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
    //!<    RAY_FLAG_CULL_BACK_FACING_TRIANGLES, RAY_FLAG_CULL_FRONT_FACING_TRIANGLES, RAY_FLAG_CULL_OPAQUE, RAY_FLAG_CULL_NON_OPAQUE = 0x80,
    //!<    RAY_FLAG_SKIP_TRIANGLES, RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES ... �v DXR1.1
    TraceRay(TLAS, 
            RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_CULL_BACK_FACING_TRIANGLES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH  ,
            0xff,
            0, 1,   //!< RayContributionToHitGroupIndex, MultiplierForGeometryContributionToHitGroupIndex : RecordAddress = RayContributionToHitGroupIndex + MultiplierForGeometryContributionToHitGroupIndex * GeometryContributionToHitGroupIndex + instanceContributionToHitGroupIndex   
            0,      //!< MissShaderIndex
            Ray, 
            Payload);

    //!< ���ʂ������_�[�^�[�Q�b�g�� (Write to render target)
    RenderTarget[DispatchRaysIndex().xy] = float4(Payload.Color, 1.0f);
    //RenderTarget[DispatchRaysIndex().xy] = float4(UpY, 0.0f, 1.0f);
}