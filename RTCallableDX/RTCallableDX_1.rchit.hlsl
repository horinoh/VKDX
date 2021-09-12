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
    CallableDataIn Data; Data.CallableData = float3(0.0f, 0.0f, 0.0f);

    CallShader(InstanceID(), Data);

    //!< —Î
    Pay.Color = Data.CallableData * float3(0.0f, 1.0f, 0.0f);
}
