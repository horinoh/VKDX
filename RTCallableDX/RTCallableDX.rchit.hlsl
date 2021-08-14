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
    Pay.Color = float3(1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y, BITIA.barycentrics.x, BITIA.barycentrics.y);

    CallableDataIn Data; Data.CallableData = float3(0.0f, 0.0f, 0.0f);

    //!< GeometryIndex() : BLAS ì¬Žž‚Ì D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS.NumDescs
    //CallShader(GeometryIndex(), Data);
    
    //!< InstanceID() : TLAS ì¬Žž‚Ì D3D12_RAYTRACING_INSTANCE_DESC.InstanceID
    CallShader(InstanceID(), Data);

    Pay.Color = Data.CallableData * float3(1.0f, 0.0f, 0.0f);
}
