struct Payload
{
    float3 Color;
};
struct CallableDataIn
{
    float3 CallableData;
};

[shader("closesthit")]
void OnClosestHit_0(inout Payload Pay, in BuiltInTriangleIntersectionAttributes BITIA)
{
    CallableDataIn Data; Data.CallableData = float3(0.0f, 0.0f, 0.0f);

    //!< [C++] D3D12_RAYTRACING_INSTANCE_DESC.InstanceID, [HLSL] InstanceID()
    CallShader(InstanceID(), Data);

    //!< Ô
    Pay.Color = Data.CallableData * float3(1.0f, 0.0f, 0.0f);
}
