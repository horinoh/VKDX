struct AttrPNT
{
    float3 Position;
    float3 Normal;
    float2 Texcoord;
};

float3 GetPosition(const float t)
{
    return ObjectRayOrigin() + ObjectRayDirection() * t;
}

bool Trace_Sphere(const float3 Center, const float Radius, out float t)
{
    const float3 Tmp0 = ObjectRayOrigin() - Center;
    const float3 d = ObjectRayDirection();
    
    const float A = dot(d, d);
    const float B2 = dot(Tmp0, d);
    const float C = dot(Tmp0, Tmp0) - Radius * Radius;
    const float D4 = B2 * B2 - A * C;
    
    t = 0.0f;
    if (D4 >= 0.0f) {
        const float Sq = sqrt(D4);
        const float Tmp1 = (-B2 - Sq) / A;
        const float Tmp2 = (-B2 + Sq) / A;
        const float t0 = min(Tmp1, Tmp2);
        t = (t0 >= RayTMin() && t0 <= RayTCurrent()) ? t0 : max(Tmp1, Tmp2);
        if (t >= RayTMin() && t <= RayTCurrent()) {
            return true;
        }
    }
    
    return false;
}
float3 GetNormal_Sphere(const float3 Center, const float3 Pos)
{
    return normalize(Pos - Center);
}
float2 GetTexcoord_Sphere(const float3 Nrm)
{
    const float PI = 4.0f * atan(1.0f);
    const float PI2 = PI * 2;
    return float2(frac(atan2(Nrm.y, Nrm.x) / PI2), acos(-Nrm.z) / PI);
}

bool Trace_AABB(const float3 Center, const float3 Radius, out float t)
{
    const float3 AABBMin = Center - Radius;
    const float3 AABBMax = Center + Radius;

    const float3 invd = 1.0f / ObjectRayDirection();
    const float3 Tmp0 = (AABBMin - ObjectRayOrigin()) * invd;
    const float3 Tmp1 = (AABBMax - ObjectRayOrigin()) * invd;
    const float3 tMin = min(Tmp0, Tmp1);
    const float3 tMax = max(Tmp0, Tmp1);
    const float t0 = max(max(tMin.x, tMin.y), tMin.z);
    const float t1 = min(min(tMax.x, tMax.y), tMax.z);
	t = 0.0f;
    if (t0 <= t1) {
        t = t0 < RayTMin() ? t1 : t0;
        if (t >= RayTMin() && t <= RayTCurrent()) {
            return true;
        }
    }
    return false;
}
float3 GetNormal_AABB(const float3 Center, const float3 Pos)
{
    const float3 N = normalize(Pos - Center);
    const float3 NAbs = abs(N);
    const float MaxComp = max(max(NAbs.x, NAbs.y), NAbs.z);
    if (MaxComp == NAbs.x)
    {
        return float3(sign(N.x), 0.0f, 0.0f);
    }
    else if (MaxComp == NAbs.y)
    {
        return float3(0.0f, sign(N.y), 0.0f);
    }
    else
    {
        return float3(0.0f, 0.0f, sign(N.z));
    }
}
float2 GetTexcoord_AABB(const float3 Center, const float3 Radius,const float3 Pos, const float3 Nrm)
{
    const float3 NAbs = abs(normalize(Pos - Center));
    const float MaxComp = max(max(NAbs.x, NAbs.y), NAbs.z);
    const float3 AABBMin = Center - Radius;
    if (MaxComp == NAbs.x)
    {
        return (float2(Nrm.x, 1.0) * Pos.yz - AABBMin.yz) / Radius.yz;
    }
    else if (MaxComp == NAbs.y)
    {
        return (float2(-Nrm.y, 1.0) * Pos.xz - AABBMin.xz) / Radius.xz;
    }
    else
    {
        return (float2(Nrm.z, 1.0) * Pos.xy - AABBMin.xy) / Radius.xy;
    }
}

//!< ペイロードへ書き込みはできない、アトリビュートを生成して他シェーダへ供給
[shader("intersection")]
void OnIntersection()
{
#if 0
    const float3 Center = float3(0.0f, 0.0f, 0.0f);
    const float3 Radius = float3(0.5f, 0.5f, 0.5f);
    float t;
    if (Trace_AABB(Center, Radius, t))
    {
        AttrPNT Attr;
        Attr.Position = GetPosition(t);
        Attr.Normal = GetNormal_AABB(Center, Attr.Position);
        Attr.Texcoord = GetTexcoord_AABB(Center, Radius, Pos, Attr.Normal);
        
        const uint Kind = 0;
        ReportHit(t, Kind, Attr);
    }
#else
    const float3 Center = float3(0.0f, 0.0f, 0.0f);
    const float Radius = 0.5f;
    float t;
    if (Trace_Sphere(Center, Radius, t)) {
        AttrPNT Attr;
        Attr.Position = GetPosition(t);
        Attr.Normal = GetNormal_Sphere(Center, Attr.Position);
        Attr.Texcoord = GetTexcoord_Sphere(Attr.Normal);
        
        const uint Kind = 0;
        ReportHit(t, Kind, Attr);
    }
#endif
}
