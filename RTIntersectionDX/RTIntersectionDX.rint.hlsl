struct AttrNT
{
    float3 Normal;
    float2 Texcoord;
};

#define USE_DISTANCE_FUNCTION

float3 GetPosition(const float t)
{
    return ObjectRayOrigin() + ObjectRayDirection() * t;
}

#ifdef USE_DISTANCE_FUNCTION
//!< 距離関数 (DistanceFunction) https://iquilezles.org/articles/distfunctions/
float DFSphere(const float3 Pos, const float3 Center, const float Radius)
{
    return length(Pos - Center) - Radius;
}
float3 DFSphereNormal(const float3 Pos, const float3 Center, const float Radius)
{
    const float Epsilon = 0.0001f;
    const float3 Ex = float3(Epsilon, 0.0f, 0.0f), Ey = float3(0.0f, Epsilon, 0.0f), Ez = float3(0.0f, 0.0f, Epsilon);
    return normalize(float3(DFSphere(Pos + Ex, Center, Radius) - DFSphere(Pos - Ex, Center, Radius), DFSphere(Pos + Ey, Center, Radius) - DFSphere(Pos - Ey, Center, Radius), DFSphere(Pos + Ez, Center, Radius) - DFSphere(Pos - Ez, Center, Radius)));
}
float DFTorus(const float3 Pos, const float3 Center, const float Radius, const float Width)
{
    const float3 d = Pos - Center;
    return length(float2(length(d.xz) - Radius, d.y)) - Width;
}
float3 DFTorusNormal(const float3 Pos, const float3 Center, const float Radius, const float Width)
{
    const float Epsilon = 0.0001f;
    const float3 Ex = float3(Epsilon, 0.0f, 0.0f), Ey = float3(0.0f, Epsilon, 0.0f), Ez = float3(0.0f, 0.0f, Epsilon);
    return normalize(float3(DFTorus(Pos + Ex, Center, Radius, Width) - DFTorus(Pos - Ex, Center, Radius, Width), DFTorus(Pos + Ey, Center, Radius, Width) - DFTorus(Pos - Ey, Center, Radius, Width), DFTorus(Pos + Ez, Center, Radius, Width) - DFTorus(Pos - Ez, Center, Radius, Width)));
}
float DFBox(const float3 Pos, const float3 Center, const float3 Radius)
{
    return length(max(abs(Pos - Center) - Radius, 0.0f));
}
float3 DFBoxNormal(const float3 Pos, const float3 Center, const float3 Radius)
{
    const float Epsilon = 0.0001f;
    const float3 Ex = float3(Epsilon, 0.0f, 0.0f), Ey = float3(0.0f, Epsilon, 0.0f), Ez = float3(0.0f, 0.0f, Epsilon);
    return normalize(float3(DFBox(Pos + Ex, Center, Radius) - DFBox(Pos - Ex, Center, Radius), DFBox(Pos + Ey, Center, Radius) - DFBox(Pos - Ey, Center, Radius), DFBox(Pos + Ez, Center, Radius) - DFBox(Pos - Ez, Center, Radius)));
}

#else

bool Trace_Sphere(const float3 Center, const float Radius, out float t)
{
    //!< レイのパラメータ表現 Ray = o + d * t ただし o = ObjectRayOrigin(), d = ObjectRayDirection()
    //!< (Ray - Center)^2 = Radius^2
    //!< (d * t + Tmp0)^2 = Radius^2 ただし Tmp0 = (o - Center)
    //!< d^2 * t^2 + 2 * Tmp0 * d * t + Tmp0^2 - Radius^2 = 0
    //!< A * t^2 + B * t + C = 0 ただし A = d^2, B = 2 * Tmp0 * d, C = Tmp0^2 - Radius^2
    //!< 判別式 D = B * B - 4.0f * A * C
    //!< 判別式 D4 = B2 * B2 - A * C ただし B2 = B / 2
    //!< 解の公式 t = (-B +- sqrt(D)) / 2 * A
    //!< 解の公式 t = (-B2 +- sqrt(D4)) / A

    const float3 Tmp0 = ObjectRayOrigin() - Center;
    const float3 d = ObjectRayDirection();
    
    const float A = dot(d, d);
    const float B2 = dot(Tmp0, d);
    const float C = dot(Tmp0, Tmp0) - Radius * Radius;
    const float D4 = B2 * B2 - A * C;
        
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
    //!< レイのパラメータ表現 Ray = o + d * t ただし o = ObjectRayOrigin(), d = ObjectRayDirection()
    //!< Ray = Plane[+-X+-Y+-Z] ... レイに 6 平面を代入
    //!< o + d * t = Plane[+-X+-Y+-Z]
    //!< t = (Plane[+-X+-Y+-Z] - o) / d
    
    const float3 AABBMin = Center - Radius;
    const float3 AABBMax = Center + Radius;

    const float3 invd = 1.0f / ObjectRayDirection();
    const float3 Tmp0 = (AABBMin - ObjectRayOrigin()) * invd;
    const float3 Tmp1 = (AABBMax - ObjectRayOrigin()) * invd;
    const float3 tMin = min(Tmp0, Tmp1);
    const float3 tMax = max(Tmp0, Tmp1);
    const float t0 = max(max(tMin.x, tMin.y), tMin.z);
    const float t1 = min(min(tMax.x, tMax.y), tMax.z);
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
#endif

//!< ペイロードへ書き込みはできない、アトリビュートを生成して他シェーダへ供給
[shader("intersection")]
void OnIntersection()
{
#ifdef USE_DISTANCE_FUNCTION
    const float Threshold = 0.0001f;
    const uint MaxSteps = 256;

    const float3 Center = float3(0.0f, 0.0f, 0.0f);
    const float Radius = 0.25f;
    
    uint i = 0;
    float t = RayTMin();
    while (i++ < MaxSteps && t <= RayTCurrent())
    {
        const float3 Pos = GetPosition(t);
        const float Distance = DFTorus(Pos, Center, Radius, Radius);
        if (Distance < Threshold)
        {
            AttrNT Attr;
            Attr.Normal = DFTorusNormal(Pos, Center, Radius, Radius);
            
            const uint Kind = 0; //!< ここでは使用しないので 0
            ReportHit(t, Kind, Attr);
            return;
        }
        t += Distance;
    }
#else
#if 0
    const float3 Center = float3(0.0f, 0.0f, 0.0f);
    const float3 Radius = float3(0.5f, 0.5f, 0.5f);
    float t;
    if (Trace_AABB(Center, Radius, t))
    {
        AttrNT Attr;
        const float3 Pos = GetPosition(t);
        Attr.Normal = GetNormal_AABB(Center, Pos);
        Attr.Texcoord = GetTexcoord_AABB(Center, Radius, Pos, Attr.Normal);
        
        const uint Kind = 0; //!< ここでは使用しないので 0
        ReportHit(t, Kind, Attr);
    }
#else
    const float3 Center = float3(0.0f, 0.0f, 0.0f);
    const float Radius = 0.5f;
    float t;
    if (Trace_Sphere(Center, Radius, t)) {
        AttrNT Attr;
        const float3 Pos = GetPosition(t);
        Attr.Normal = GetNormal_Sphere(Center, Pos);
        Attr.Texcoord = GetTexcoord_Sphere(Attr.Normal);
        
        const uint Kind = 0; //!< ここでは使用しないので 0
        ReportHit(t, Kind, Attr);
    }
#endif
#endif
}
