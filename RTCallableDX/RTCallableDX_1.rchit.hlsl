struct PAYLOAD
{
    float3 Color;
    int Recursive;
};
struct CallableDataIn
{
    float3 CallableData;
};

[shader("closesthit")]
void OnClosestHit_1(inout PAYLOAD Payload, in BuiltInTriangleIntersectionAttributes BITIA)
{
    if (Payload.Recursive++ >= 1)
    {
        Payload.Color = float3(0.0f, 1.0f, 0.0f);
        return;
    }
    
    CallableDataIn Data; Data.CallableData = float3(0.0f, 0.0f, 0.0f);

    CallShader(InstanceID(), Data);

    //!< ��
    Payload.Color = Data.CallableData * float3(0.0f, 1.0f, 0.0f);
}
