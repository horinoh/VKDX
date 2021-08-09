//StructuredBuffer<float3> VertexBuffer : reister(t0, space0)
//StructuredBuffer<uint> IndexBuffer : reister(t1, space0)

struct Payload
{
    float3 Color;
};
struct CallableDataIn
{
    float3 CallableData;
};

[shader("closesthit")]
void OnClosestHit(inout Payload Pay, in BuiltInTriangleIntersectionAttributes BITIA)
{
    //InstanceID() : TLAS çÏê¨éûÇÃ D3D12_RAYTRACING_INSTANCE_DESC.InstanceID
    Pay.Color = float3(1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y, BITIA.barycentrics.x, BITIA.barycentrics.y);

    CallableDataIn Data; Data.CallableData = float3(0.0f, 0.0f, 0.0f);
    //CallShader(GeometryIndex(), Data);
    const uint ShaderIndex = 0; //!< Ç±Ç±Ç≈ÇÕ [0, 2]
    CallShader(ShaderIndex, Data);
    Pay.Color = Data.CallableData;
}
