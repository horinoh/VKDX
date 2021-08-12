struct Payload
{
    float3 Color;
};
struct CallableDataIn
{
    float3 CallableData;
};

[shader("closesthit")]
void OnClosestHit_1(inout Payload Pay, in BuiltInTriangleIntersectionAttributes BITIA)
{
    Pay.Color = float3(1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y, BITIA.barycentrics.x, BITIA.barycentrics.y);

    CallableDataIn Data; Data.CallableData = float3(0.0f, 0.0f, 0.0f);

    CallShader(InstanceID(), Data);

    Pay.Color = Data.CallableData * float3(0.0f, 1.0f, 0.0f);
}
