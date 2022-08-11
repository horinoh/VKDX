struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

[shader("closesthit")]
void OnClosestHit(inout PAYLOAD Payload, in BuiltInTriangleIntersectionAttributes BITIA)
{
    if (Payload.Recursive++ >= 1) {
        Payload.Color = float3(0.0f, 0.0f, 0.0f);
        return;
    }
    //!< v0, v1, v2 �̂R���_�̏ꍇ�ABuiltInTriangleIntersectionAttributes.barycentris.xy �͂��ꂼ�� v1, v2 �̃E�G�C�g
    //!< v = v0 + BuiltInTriangleIntersectionAttributes.barycentrics.x * (v1 - v0) + BuiltInTriangleIntersectionAttributes.barycentrics.y * (v2 - v0)
    Payload.Color = float3(1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y, BITIA.barycentrics.x, BITIA.barycentrics.y); //!< BaryCentric
}
