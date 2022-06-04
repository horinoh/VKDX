struct Payload
{
    float3 Color;
};

[shader("closesthit")]
void OnClosestHit(inout Payload Pay, in BuiltInTriangleIntersectionAttributes BITIA)
{
    //!< v0, v1, v2 �̂R���_�̏ꍇ�ABuiltInTriangleIntersectionAttributes.barycentris.xy �͂��ꂼ�� v1, v2 �̃E�G�C�g
    //!< v = v0 + BuiltInTriangleIntersectionAttributes.barycentrics.x * (v1 - v0) + BuiltInTriangleIntersectionAttributes.barycentrics.y * (v2 - v0)
    Pay.Color = float3(1.0f - BITIA.barycentrics.x - BITIA.barycentrics.y, BITIA.barycentrics.x, BITIA.barycentrics.y); //!< BaryCentric
}
