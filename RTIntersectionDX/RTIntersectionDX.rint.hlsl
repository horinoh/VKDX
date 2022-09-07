struct VertexNT
{
    float3 Normal;
    float2 Texcoord;
};

static const float PI = 4.0f * atan(1.0f);
static const float PI2 = PI * 2.0f;

//!< ペイロードへ書き込みはできない、アトリビュートを生成して他シェーダへ供給
[shader("intersection")]
void OnIntersection()
{
    const float3 Center = float3(0.0f, 0.0f, 0.0f);
    const float Radius = 0.25f;
    
    //!< レイのパラメータ表現 Ray = o + d * t ただし o = ObjectRayOrigin(), d = ObjectRayDirection()
    //!< (Ray - Center)^2 = Radius^2
    //!< (d * t + Tmp)^2 = Radius^2 ただし Tmp = (o - Center)
    //!< d^2 * t^2 + 2 * Tmp * d * t + Tmp^2 - Radius^2 = 0
    //!< A * t^2 + B * t + C = 0 ただし A = d^2, B = 2 * Tmp * d, C = Tmp^2 - Radius^2
    //!< 判別式 D = B * B - 4.0f * A * C
    //!< 判別式 D4 = B2 * B2 - A * C ただし B2 = B / 2
    //!< 解の公式 t = (-B +- sqrt(D)) / 2 * A
    //!< 解の公式 t = (-B2 +- sqrt(D4)) / A

    const float3 Tmp = ObjectRayOrigin() - Center;
    const float3 d = ObjectRayDirection();
    
    const float A = dot(d, d);
    const float B = 2.0f * dot(Tmp, d);
    const float B2 = dot(Tmp, d);
    const float C = dot(Tmp, Tmp) - Radius * Radius;
    const float D = B * B - 4.0f * A * C;
    const float D4 = B2 * B2 - A * C;
        
    if (D4 >= 0.0f) {    
        //const float t0 = (-B - sqrt(D)) / (A * 2.0f);
        //const float t1 = (-B + sqrt(D)) / (A * 2.0f);
        
        const float Sq = sqrt(D4);
        const float t0 = (-B2 - Sq) / A;
        const float t1 = (-B2 + Sq) / A;
        const float t = 0;
        
        const float3 Hit = ObjectRayOrigin() + ObjectRayDirection() * t;
        
        VertexNT Attr;
        Attr.Normal = normalize(Hit - Center);
        const float3 LocalN = mul(WorldToObject3x4(), float4(Attr.Normal, 0.0f));
        Attr.Texcoord = float2(frac(atan2(LocalN.y, LocalN.x) / PI2), acos(-LocalN.z) / PI);
        
        const uint Kind = 0; //!< ここでは使用しないので 0
        //!< ヒットした場合のみ
        ReportHit(t0, Kind, Attr);
    }
}
