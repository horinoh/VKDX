//StructuredBuffer<float3> VertexBuffer : reister(t0, space0)
//StructuredBuffer<uint> IndexBuffer : reister(t1, space0)

struct Payload
{
    float3 Color;
};

[shader("closesthit")]
void OnClosestHit(inout Payload Pay, in BuiltInTriangleIntersectionAttributes BITIA)
{
    //InstanceID() : TLAS çÏê¨éûÇÃ D3D12_RAYTRACING_INSTANCE_DESC.InstanceID
    Pay.Color = float3(1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y, BITIA.barycentrics.x, BITIA.barycentrics.y);
}
