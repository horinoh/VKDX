struct PAYLOAD
{
    float3 Color;
    int Recursive;
};
struct CALLABLEDATA
{
    float3 Data;
};

[shader("closesthit")]
void OnClosestHit_0(inout PAYLOAD Payload, in BuiltInTriangleIntersectionAttributes BITIA)
{
    if (Payload.Recursive++ >= 1)
    {
        Payload.Color = float3(0.0f, 1.0f, 0.0f);
        return;
    }
    
    CALLABLEDATA CallableData;
    CallableData.Data = float3(0.0f, 0.0f, 0.0f);

    //!< [C++] D3D12_RAYTRACING_INSTANCE_DESC.InstanceID, [HLSL] InstanceID()
    CallShader(InstanceID(), CallableData);

    //!< Ô
    Payload.Color = CallableData.Data * float3(1.0f, 0.0f, 0.0f);
}
