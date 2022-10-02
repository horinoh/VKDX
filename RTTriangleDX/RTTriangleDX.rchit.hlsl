struct PAYLOAD
{
    float3 Color;
    int Recursive;
};

//!< v0, v1, v2 の３頂点の場合、BuiltInTriangleIntersectionAttributes.barycentris.xy はそれぞれ v1, v2 のウエイト
float3 ToBaryCentric(const float2 Attr)
{
    return float3(1.0f - Attr.x - Attr.y, Attr.x, Attr.y);
}
//!< v = v0 + BuiltInTriangleIntersectionAttributes.barycentrics.x * (v1 - v0) + BuiltInTriangleIntersectionAttributes.barycentrics.y * (v2 - v0)
float3 ToCartesian(const float3 A, const float3 B, const float3 C, const float3 BaryCentric)
{
    return BaryCentric.x * A + BaryCentric.y * B + BaryCentric.z * C;
}
float3 ToCartesian(const float3 V[3], const float3 BaryCentric)
{
    return ToCartesian(V[0], V[1], V[2], BaryCentric);
}

[shader("closesthit")]
void OnClosestHit(inout PAYLOAD Payload, in BuiltInTriangleIntersectionAttributes BITIA)
{
    if (Payload.Recursive++ >= 1) {
        Payload.Color = float3(0.0f, 1.0f, 0.0f);
        return;
    }
    Payload.Color = ToBaryCentric(BITIA.barycentrics);

}
